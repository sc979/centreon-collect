from robot.api import logger
from subprocess import getoutput
import re
import time
from dateutil import parser
from datetime import datetime

TIMEOUT = 30


def check_connection(port: int, pid1: int, pid2: int):
    limit = time.time() + TIMEOUT
    r = re.compile(r"^ESTAB.*127\.0\.0\.1:{}\s".format(port))
    while time.time() < limit:
        out = getoutput("ss -plant")
        lst = out.split('\n')
        estab_port = list(filter(r.match, lst))
        if len(estab_port) >= 2:
            ok = [False, False]
            p = re.compile(r"127\.0\.0\.1:(\d+)\s+127\.0\.0\.1:(\d+)\s+.*,pid=(\d+)")
            for l in estab_port:
                m = p.search(l)
                if m:
                    if pid1 == int(m.group(3)):
                        ok[0] = True
                    if pid2 == int(m.group(3)):
                        ok[1] = True
            if ok[0] and ok[1]:
                return True
        time.sleep(1)

    return False


def get_date(d: str):
    try:
        ts = int(d)
        retval = datetime.fromtimestamp(int(ts))
    except ValueError:
        retval = parser.parse(d[:-6])
    return retval


def find_in_log_with_timeout(log: str, date, content, timeout: int):
    limit = time.time() + TIMEOUT
    while time.time() < limit:
        if find_in_log(log, date, content):
            return True
        time.sleep(5)
    return False


def find_in_log(log: str, date, content):
    my_date = parser.parse(date)
    try:
        f = open(log, "r")
        lines = f.readlines()
        p = re.compile(r"\[([^\]]*)\]")

        # Let's find my_date
        start = 0
        end = len(lines) - 1
        idx = start
        while end - start > 1:
            idx = (start + end) // 2
            m = p.match(lines[idx])
            idx_d = get_date(m.group(1))
            if my_date <= idx_d:
                end = idx
            elif my_date > idx_d:
                start = idx

        for c in content:
            found = False
            for i in range(idx, len(lines)):
                line = lines[i]
                if c in line:
                    logger.console("\"{}\" found at line {} from {}".format(c, i, idx))
                    found = True
                    break
            if not found:
                logger.console("Unable to find from {} (from line {}): {}".format(date, idx, c))
                return False

        return True
    except IOError:
        logger.console("The file '{}' does not exist".format(log))
        return False


def get_hostname():
    retval = getoutput("hostname --fqdn")
    retval = retval.rstrip()
    return retval


def create_key_and_certificate(host: str, key: str, cert: str):
    if len(key) > 0:
        retval = getoutput(
            "openssl req -new -newkey rsa:2048 -days 365 -nodes -x509 -keyout {} -out {} -subj '/CN={}'".format(key,
                                                                                                                cert,
                                                                                                                host))
    else:
        retval = getoutput(
            "openssl req -new -newkey rsa:2048 -days 365 -nodes -x509 -out {} -subj '/CN={}'".format(key, cert, host))


def create_certificate(host: str, cert: str):
    create_key_and_certificate(host, "", cert)


def start_mysql():
    getoutput("systemctl start mysql")


def stop_mysql():
    getoutput("systemctl stop mysql")


def kill_broker():
    getoutput("kill -SIGTERM $(ps aux | grep '/usr/sbin/cbwd' | grep -v grep | awk '{print $2}')")
    getoutput("kill -SIGTERM $(ps aux | grep '/usr/sbin/cbd' | grep -v grep | awk '{print $2}')")


def kill_engine():
    getoutput("kill -SIGTERM $(ps aux | grep '/usr/sbin/centengine' | grep -v grep | awk '{print $2}')")


def clear_retention():
    getoutput("find /var -name '*.cache.*' -delete")
    getoutput("find /tmp -name 'lua*' -delete")
    getoutput("find /tmp -name 'central-*' -delete")
    getoutput("find /var -name '*.memory.*' -delete")
    getoutput("find /var -name '*.queue.*' -delete")
    getoutput("find /var -name '*.unprocessed*' -delete")


def clear_cache():
    getoutput("find /var -name '*.cache.*' -delete")

