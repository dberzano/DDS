#! /usr/bin/env bash

location=$1
echo "DDS from this location is used ${location}"
source ${location}/DDS_env.sh

sshFileTemplate=${DDS_LOCATION}/tests/property_test_hosts.cfg
sshFile=${DDS_LOCATION}/tests/property_test_hosts_real.cfg
topologyFile=${DDS_LOCATION}/tests/property_test.xml
wrkDir=${DDS_LOCATION}/tests/tmp/wn_test_dir/
requiredNofAgents=10

mkdir -p ${wrkDir}

# Replace __WRK_DIR__ in SSH file with real directory
sedDir="${wrkDir//\//\\/}"
sed -e "s/__WRK_DIR__/${sedDir}/" ${sshFileTemplate} > ${sshFile}

echo "Starting DDS server..."
dds-server restart -s

echo "Submiting agents..."
dds-submit -r ssh --config ${sshFile}

sleep 5

nofAgents=$(dds-info -n)
while [  ${nofAgents} -lt ${requiredNofAgents} ]; do
	nofAgents=$(dds-info -n)
done

echo "Activating topology..."
dds-topology --activate ${topologyFile}

# DDS commander updates property list each 60 seconds
echo "Waiting 60 seconds..."
sleep 60

nofGoodResults=$(grep "User task exited with status 0" ${wrkDir}/wn_property*/*.log | wc -l)

if [ "${nofGoodResults}" -eq "${requiredNofAgents}" ]
then
  echo "Test passed."
else
  echo "Test failed: number of good results is ${nofGoodResults}, required number of good results is ${requiredNofAgents}"
fi

echo "Stoping server..."
dds-server stop

rm ${sshFile}
rm -rf ${wrkDir}

exit 0
