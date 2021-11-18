rmq_server_id=$(docker ps | grep "rabbitmq" | awk '{print $1}')

docker kill $rmq_server_id

docker compose up -d