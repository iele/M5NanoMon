import time
import json
import serial
import psutil
import shutil
import GPUtil
import signal

def format_number(num):
    num = int(num)
    if abs(num) < 1000:
        return f"{num:05.1f}B"
    elif abs(num) < 1000**2:
        return f"{num / 1000:05.1f}K"
    elif abs(num) < 1000**3:
        return f"{num / 1000**2:05.1f}M"
    elif abs(num) < 1000**4:
        return f"{num / 1000**3:05.1f}G"
    else:
        return f"{num / 1000**4:05.1f}T"


# MAC: DEVICE = 'dev/tty.devicename'
# WINDOWS: Device = 'COM(port here)'
DEVICE = "COM15"

count = 0  # to see how many times data has been sent

old_time_ns = time.time_ns()
net_stat = psutil.net_io_counters()
old_net_send = net_stat.bytes_sent
old_net_recv = net_stat.bytes_recv

while True:
    try:
        ser = serial.Serial(port=DEVICE, baudrate=115200, write_timeout=1)  # open serial port

        while True:
            try:
                count = count + 1
                # CPU Stats
                cpu_percent = psutil.cpu_percent(interval=1) # Get CPU percent (Takes 1 second)
                cpu_freq = psutil.cpu_freq().current
                # RAM Stats
                ram_percent = psutil.virtual_memory().percent
                ram_used = format_number(psutil.virtual_memory().used)
                ram_total = format_number(psutil.virtual_memory().total)
                # Get Disk Stats
                storage_total, storage_used, storage_free = shutil.disk_usage("/")
                disk_percent = round(
                    (storage_used / (2**30)) / (storage_total / (2**30)) * 100, 1
                )
                disk_total = format_number(storage_total)
                disk_used = format_number(storage_used)
                time_ns = time.time_ns()
                net_stat = psutil.net_io_counters()
                net_send = net_stat.bytes_sent
                net_recv = net_stat.bytes_recv
                net_send_rate = (net_send - old_net_send) * (10**9) / (time_ns - old_time_ns)
                net_recv_rate = (net_recv - old_net_recv) * (10**9) / (time_ns - old_time_ns)

                old_net_send = net_send
                old_net_recv = net_recv
                old_time_ns = time_ns

                # Create JSON Data
                command = {
                    "cpu": {"percent": cpu_percent, "cpu_freq": cpu_freq},
                    "ram": {"percent": ram_percent, "used": ram_used, "total": ram_total},
                    "disk": {"percent": disk_percent, "used": disk_used, "total": disk_total},
                    "net": {
                        "send": format_number(net_send_rate), "recv": format_number(net_recv_rate),
                        "send_orig": int(net_send_rate), "recv_orig": int(net_recv_rate),
                    },
                }
                command_json = json.dumps(command) + "\n"
                command_json = command_json.encode("utf-8")  # Format the data
                print(f"Sending Data... | x{count} \n {command_json}\n")
                ser.write(command_json)  # Write a string to the serial
                print(f"Receive Data... | x{count} \n ", ser.read_all())
            except Exception as e:
                print(e)
                break

    except Exception as e:
        print(e)
        print("Restarting... \n")
        time.sleep(10)