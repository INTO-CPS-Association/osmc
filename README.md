# osmc
Out-of-sync Mitigation Component

## Dependencies:
thirdparty/restinio: Restinio v.0.6.13

## Setup rabbitmq server with docker, and run helper script
```bash
$ cd case-study/server
$ ./setup.sh
$ python3 playback_gazebo_data-test.py
```

## Run test with mvn:

```bash
$ mvn test -Dtest=Test#initialTest -DfailIfNoTests=false 
```
