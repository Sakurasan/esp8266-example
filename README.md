# esp8266-example

dht11 
  - GPIO2引脚 接 D4

oled 
  - scl 接 D2
  - sda 接 D1

docker run --init -d --name="HA" -v $PWD:/config -v /etc/localtime:/etc/localtime:ro -p 8123:8123 homeassistant/home-assistant