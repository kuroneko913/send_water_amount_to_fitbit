# send_water_amount_to_fitbit
飲んだ水の量をFitbitAPIで連携して記録するシステム。

![image](https://user-images.githubusercontent.com/20674685/146415672-ab0f6ceb-f8d2-4621-a3ab-5782d58a2e74.png)

M5Stack側にwater_weight.inoを書き込む。(WiFiの設定を追加して)

RaspberryPi3にraspi-mqtt-client/以下を配置する。

paho-mqtt
https://www.eclipse.org/paho/index.php?page=clients/python/index.php 

mosquittoでMQTTブローカーを立ち上げておく。

```
sudo apt install mosquitto mosquitto-clients
sudo service mosquitto start
```

ちゃんとブローカーが立ち上がっているかをsubscriberを指定して、M5Stack側(publisher)からメッセージを送信する。
```
mosquitto_sub -d -t /pub/M5Stack
```

ラズパイ側で以下の処理を実行し、MQTTクライアント機能を起動する。
```
pipenv install 
pipenv run python mqtt-to-fitbit.py 
```

M5StackからBボタン長押しで、publishedすると以下のようにMQTT経由でメッセージが送信されていることを確認できる。
```
user@RadiberryPi3:~/send_water $ pipenv run python mqtt-to-fitbit.py
Connected with result code: 0
Received message 'b'{"water_weight": 49}'' on topic '/pub/M5Stack' with QoS 0
```

Fitbitのダッシュボードから飲んだ水の量が連携されていることを確認できる。
![IMG_AADC7844C77A-1](https://user-images.githubusercontent.com/20674685/146418922-078b0105-b0af-4e6e-8832-86f3d01b1d50.jpeg)
