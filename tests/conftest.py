import time
import pytest
import serial

N = 10

def pytest_addoption(parser):
    parser.addoption("--port", required=True)

def split_line(line):
    strp = line.split()
    result = {"type": strp[0]}
    for i in strp[1:]:
        k, sep, v = i.partition("=")
        if sep == "":
            continue
        result[k] = int(v, 0)
    return result

@pytest.fixture(scope="session")
def meas(request):
    port = request.config.getoption("--port")
    result = []
    with serial.Serial(port, 115200, timeout=2) as ser:
        for _ in range(N + 5):              # a few extra reads for cut-off lines
            line = ser.readline().decode("ascii", errors="replace").strip()
            strp = line.split()
            if not strp:
                continue
            if strp[0] not in ("MEAS", "ERROR"):
                continue
            result.append(split_line(line))
            if len(result) == N:
                break
    return result