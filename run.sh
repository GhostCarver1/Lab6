gcc client.c package.c file_manager.c -o client ; gcc file_manager.c server.c package.c -o server
PID=$(sudo lsof -t -i :8080)

if [ -n "$PID" ]; then
    sudo kill -9 $PID
fi

./server 

#command to make run.sh executable