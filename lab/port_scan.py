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


def scan(ip, q_done):
    if not os.path.exists(ip):
        os.system(f"mkdir {ip}")
    cmd = (
        f"sudo nmap {ip} -Pn -p 1-65535 -sS -T4 -O --open --reason -oN ./{ip}/ports.txt"
    )
    try:
        rs = os.system(cmd)
    except Exception as e:
        pass
    q_done.put(port)


def checkstatus(q_todo, q_done):
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
        left = math.floor((30 - 10) * 0.5)
        msg = f"\r{'='*left} IP Explore {'='*left}"
        for key in res.keys():
            msg += "\n"
            ks = "IP:  " + key + " " * (17 - len(key))
            if res[key]:
                msg += f"\r{ks} Scan Finished.  [*]"
            else:
                msg += f"\r{ks} is under scaning [{array[count%4]}]"
        msg += "\n"
        msg += f"\rTime Cost: {datetime.datetime.now()-t0}"
        print(msg)
        print("\r")

        # Check if every port scan complete.
        for key in res.keys():
            is_end = is_end and res[key]
        if is_end:
            break

        time.sleep(0.2)


if __name__ == "__main__":
    # args
    parser = argparse.ArgumentParser()
    parser.add_argument("-host", "--host", help="Target Host.", type=str)
    # TODO add output path
    args = parser.parse_args()

    ips = args.host.split(",")
    q_todo = multiprocessing.Manager().Queue(len(ips))
    q_done = multiprocessing.Manager().Queue(len(ips))
    for ip in ips:
        q_todo.put(ip)
        print(ip)

    # 启一个监控的线程
    print(f"启动监控线程")
    controller = threading.Thread(
        target=checkstatus,
        args=(
            q_todo,
            q_done,
        ),
    )
    controller.start()

    p = Pool(10)
    for ip in ips:
        p.apply_async(
            scan,
            args=(ip, q_done),
        )

    p.close()
    p.join()
    print("Finished.")
