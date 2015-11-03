ulimit -c unlimited

mv robotif robotPassContent

chmod +x robotPassContent
killall -9 robotPassContent
./robotPassContent robot.conf
tail -f log/robot.log

