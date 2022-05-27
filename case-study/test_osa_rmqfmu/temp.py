import datetime
import time

start = 2
end = 6

start_time = datetime.datetime.now()

time_exit = 20
time_now = datetime.datetime.now()
while time_now < start_time + datetime.timedelta(seconds=time_exit):
    
    if time_now >= start_time + datetime.timedelta(seconds=start) and time_now <= start_time + datetime.timedelta(seconds=end):
        print("In the zone")

    time.sleep(1)
    time_now = datetime.datetime.now()