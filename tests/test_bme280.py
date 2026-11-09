# pytest tests --port /dev/tty.usbmodem144201
from conftest import N

EXPECTED_CHIP_ID = 0x60
T_MIN, T_MAX = 1000, 3500 # 0.01 degC -> 10..35 degC
P_MIN, P_MAX = 90000, 105000 # Pa -> 900..1050 hPa
H_MIN, H_MAX = 1000, 8000 # 0.01 %RH -> 10..80 %RH

def test_count(meas):
    assert len(meas) == N

def test_no_errors(meas):
    for m in meas:
        assert m["type"] == "MEAS"

def test_chip_id(meas):
    for m in meas:
        assert m["id"] == 0x60

def test_temperature_range(meas):
    for m in meas:
        assert T_MIN <= m["T"] and m["T"] <= T_MAX

def test_pressure_range(meas):
    for m in meas:
        assert P_MIN <= m["P"] and m["P"] <= P_MAX

def test_humidity_range(meas):
    for m in meas:
        assert H_MIN <= m["H"] and m["H"] <= H_MAX

def test_seq_continuous(meas):
    for i in range(len(meas) - 1):
        assert meas[i + 1]["seq"] == meas[i]["seq"] + 1

def test_not_frozen(meas):
    same_in_a_row = 0
    for i in range(len(meas) - 1):
        if meas[i + 1]["adcP"] == meas[i]["adcP"]:
            same_in_a_row += 1
        else:
            same_in_a_row = 0
        assert same_in_a_row < 3