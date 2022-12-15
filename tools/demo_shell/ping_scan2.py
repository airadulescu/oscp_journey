import sys
import subprocess


if __name__ == "__main__":
    for i in range(int(sys.argv[2]), int(sys.argv[3]) + 1):
        res = subprocess.run(
            ["ping", "-w", "500", "-c", "1", sys.argv[1] + "." + str(i)],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )
        if res.returncode == 0:
            print(sys.argv[1] + "." + str(i))
