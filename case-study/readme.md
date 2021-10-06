The co-simulation setup is as follows: the rabbitmq fmu is coupled to the osmc fmu, where the output of the osmc is fed to the rabbitmq fmu.
The configuration files are the ```multimodel.json``` and ```coe.json```.

In order to feed data to the cosimulation, the following should be setup:

```bash
cd server
docker compose up -d
python3 playback_gazebo_data-test.py
```
The script will ask for the operation mode, possible options are: normal, degrade, drop.

The script reads data from ```gazebo_playback_data-noround.csv```.
Thereafter in a separate terminal run in the case-study folder:

```bash
java -jar maestro-2.1.7-jar-with-dependencies.jar import sg1 --interpret multimodel.json coe.json -output=dump/
```

This will generate the mabl file ```spec.mabl``` in the dump folder.
This is the file we will change to implement the mitigation strategy. 