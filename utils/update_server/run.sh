docker build -t update_server .

docker run -d \
  -p 3001:3001 \
  --name update_server \
  -v "$(pwd)"/firmwares:/home/node/app/firmwares \
  -v "$(pwd)"/config:/home/node/app/config \
  update_server:latest
