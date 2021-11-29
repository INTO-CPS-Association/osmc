#!/usr/bin/env python3
import pika
import json
import datetime
import time
import csv
import argparse
import copy
import math

connection = pika.BlockingConnection(pika.ConnectionParameters('localhost'))
channel = connection.channel()
print("Declaring exchange")
channel.exchange_declare(exchange='fmi_digital_twin', exchange_type='direct')
print("Creating queue")
result = channel.queue_declare(queue='', exclusive=True)
queue_name = result.method.queue
channel.queue_bind(exchange='fmi_digital_twin', queue=queue_name,
                   routing_key='linefollower.data.from_cosim')
time_sleep = 0.1 #should match the step size

data = 'gazebo_playback_data-noround.csv'

print(' [*] Waiting for logs. To exit press CTRL+C, sleep time [ms], matching step size: ', time_sleep*1000)
EPSILON = 10 ** -12

def publish(delay_mode, start_delay, end_delay, time_delay):
    msg_array = []
    dt=datetime.datetime.strptime('2019-01-04T16:41:24+0200', "%Y-%m-%dT%H:%M:%S%z")
    print(dt)
    msg = {}
    msg['time']= dt.isoformat()
    msg['xpos']=0.0
    msg['ypos']=0.0
    i = 0.1
    with open(data, newline='') as csvfile:
        reader = csv.DictReader(csvfile)
        for row in reader:
            t = row['time']
            xpos = float(row['xpos'])
            ypos = float(row['ypos'])
            msg['xpos']=xpos
            msg['ypos']=ypos           
            msg['seqno']=int(i*10)
            timet = datetime.datetime.strptime(t, "%Y-%m-%dT%H:%M:%S.%f%z")
            msg['time']= timet.isoformat()

            if delay_mode == "degrade":
                if i >= start_delay and i < end_delay:
                    print(" [DEGRADED] In degraded mode with delay %f" % time_delay)
                    time.sleep(time_delay)
                else:
                    time.sleep(time_sleep)
                print(" [x] Sent %s" % json.dumps(msg))
                channel.basic_publish(exchange='fmi_digital_twin',
                            routing_key='default',
                            body=json.dumps(msg))
                
            elif delay_mode == "drop":
                period = end_delay-start_delay
                if i >= start_delay  and i < end_delay - EPSILON:
                    msg_array.extend([copy.deepcopy(msg)])
                    time.sleep(time_sleep)
                    print(" [DROP] In drop mode with delay %f\n Messages will be sent in a burst after the delay period" % period)
                    print(" [DROP] i %f" % i)
                elif abs(i - end_delay) < EPSILON:
                    msg_array.extend([copy.deepcopy(msg)])
                    print(" [DROP END] After drop mode with delay %f\n Sending as fast as possible" % period)
                    #print(" [DROP END] Messages %s" % str(msg_array))
                    for x in msg_array:
                        print(" [DROP END] %s")
                        channel.basic_publish(exchange='fmi_digital_twin',
                            routing_key='default',
                            body=json.dumps(x))

                        print(" [DROP END] %s", json.dumps(x))
                    msg_array = []
                else:
                    print(" [OK] Sent %s" % json.dumps(msg))
                    channel.basic_publish(exchange='fmi_digital_twin',
                                routing_key='default',
                                body=json.dumps(msg))
                    time.sleep(time_sleep)
            else:
                print(" [NORMAL] Sent %s" % json.dumps(msg))
                channel.basic_publish(exchange='fmi_digital_twin',
                            routing_key='default',
                            body=json.dumps(msg))
                time.sleep(time_sleep)
            i = i + 0.1

def callback(ch, method, properties, body, delay_mode, start_delay, end_delay, time_delay):
    print(" [x] %r" % body)
    if "waiting for input data for simulation" in str(body):
      publish(delay_mode, start_delay, end_delay, time_delay)

if __name__ == '__main__':
    options = argparse.ArgumentParser(prog="playback_gazebo_data-test", epilog="""

    python playback_gazebo_data-test.py -mode [normal | degrade | drop]

    """)
    options.add_argument("-mode", dest="mode", type=str, required=False, default="normal", help='Select mode for publishing to the rmqfmu')
    args = options.parse_args()

    delay_mode = args.mode # options normal/degrade/drop
    print("Running in %s mode" % delay_mode)
    start_delay = 2 # delay starts at this step
    end_delay = 8 # delay ends at this step
    time_delay = 2 # delay between each publieshed message, bigger than the timestep, used as sleep between msg published in the degraded mode

    channel.basic_consume(
        queue=queue_name, on_message_callback=lambda ch, method, property, body: callback(ch, method, property, body, delay_mode, start_delay, end_delay, time_delay), auto_ack=True)
    channel.start_consuming()
    connection.close()