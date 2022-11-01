# coding:utf-8
import argparse
import queue
import threading
import multiprocessing
from multiprocessing import Pool
import os
import datetime
import subprocess
import time
import math


def scan(ip, port, q_done):
    cmd = f"sudo nmap {ip} -p {port} -sV -vv --open --reason --script=auth,vuln > {port}.txt"
    try:
        rs = os.system(cmd)
    except Exception as e:
        pass
    q_done.put(port)


def checkstatus(q_todo, q_done, host):
    array = ["\\", "|", "/", "-"]
    res = {}
    t0 = datetime.datetime.now()
    # Init res dict
    while True:
        try:
            port = q_todo.get(block=False)
            res[port] = False
        except queue.Empty:
            break

    # init break flag
    count = 0
    while True:
        count += 1
        is_end = True
        # If scan ends. Set res dict True
        try:
            item = q_done.get(block=False)
            res[item] = True
        except Exception as e:
            pass

        # echo
        s = ""
        os.system("clear")
        # l = 30
        host_len = len(host) + 2
        left = math.floor((30 - host_len) * 0.5)
        msg = f"\r{'='*left} {ip} {'='*(30-host_len-left)}"
        for key in res.keys():
            msg += "\n"
            ks = f"\rPort:{key}" + " " * (5 - len(key))
            if res[key]:
                msg += f"{ks}Scan Finished. [*]"
            else:
                msg += f"{ks}is under scaning [{array[count%4]}]"
        msg += "\n"
        msg += f"\rTime Cost: {datetime.datetime.now()-t0}"
        print(msg)
        print("\r")

        # Check if every port scan complete.
        for key in res.keys():
            is_end = is_end and res[key]
        if is_end:
            break

        time.sleep(0.1)


if __name__ == "__main__":
    # args
    parser = argparse.ArgumentParser()
    parser.add_argument("-host", "--host", help="Target Host.", type=str)
    # TODO add output path
    parser.add_argument(
        "-ports", "--ports", help="Target Ports, use ',' to split each port.", type=str
    )
    args = parser.parse_args()

    ip = args.host
    print(f"Target ip is {ip}")
    ports = args.ports.split(",")
    print(f"{len(ports)} Ports {ports}")
    q_todo = multiprocessing.Manager().Queue(len(ports))
    q_done = multiprocessing.Manager().Queue(len(ports))
    for port in ports:
        q_todo.put(port)

    # 启一个监控的线程
    print(f"启动监控线程")
    controller = threading.Thread(
        target=checkstatus,
        args=(
            q_todo,
            q_done,
            ip,
        ),
    )
    controller.start()

    p = Pool(10)
    for port in ports:
        p.apply_async(
            scan,
            args=(ip, port, q_done),
        )

    p.close()
    p.join()
    print("Finished.")
