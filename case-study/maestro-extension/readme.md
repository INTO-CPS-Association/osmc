#### Development Environment
You need Java 11 and maven 3.6 to build the project (??).
The project can be built from CLI using the maven commands.
```bash
mvn clean
mvn compile
mvn test
```

MaBL file for the SimpleTest.java: SmallFaultInjectTest.mabl

To run only one test: mvn test -Dtest=TestClassName#testMethod -DfailIfNoTests=false, e.g.

```bash
$ mvn test -Dtest=Test#initialTest -DfailIfNoTests=false
```

The `-DfailIfNoTests`is set to `false`, to avoid an error due to no tests in faultinject folder.


#### Running the experiment

In a separate terminal window, go to the server folder, from there:
To setup the rabbitmq server through docker
```bash
$ ./setup.sh 
```
To run the helper script (in one of three modes, normal - default, drop, and degrade)
```bash
$ python3 playback_gazebo_data-test.py -mode drop
```

In a separate terminal window, in the maestro-extention folder run the test (with osmc and rmqfmu):
Remember, it needs java 11. 
```bash
$  mvn test -Dtest=Test#initialTest -DfailIfNoTests=false
```



