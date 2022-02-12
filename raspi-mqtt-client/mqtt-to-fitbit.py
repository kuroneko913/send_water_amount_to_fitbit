# mosquitto がインストールされていることが前提。
from paho.mqtt import client as mqtt_client
import fitbit
import json
import os
import requests
import base64
from datetime import datetime

broker = 'localhost'
port = 1883
topic = '/pub/M5Stack'

# https://dev.fitbit.com/apps
CLIENT_ID = "fitbit-app-client-id"
CLIENT_SECRET = "fitbit-app-client-secret"
REFRESH_TOKEN = "refresh-token"

def on_connect(client, userdata, flag, rc):
    print(f"Connected with result code: {rc}")
    client.subscribe(topic)

# ブローカーが切断したときの処理
def on_disconnect(client, userdata, flag, rc):
  if  rc != 0:
    print("Unexpected disconnection.")

# メッセージが届いたときの処理
def on_message(client, userdata, msg):
  # msg.topicにトピック名が，msg.payloadに届いたデータ本体が入っている
  print("Received message '" + str(msg.payload) + "' on topic '" + msg.topic + "' with QoS " + str(msg.qos))
  tokens = get_tokens()
  if ('refresh_token' in tokens):
    save_tokens(tokens)
  call_api(tokens, json.loads(msg.payload))

def update_tokens(refresh_token):
  basic_token = bytes(f'{CLIENT_ID}:{CLIENT_SECRET}','utf-8')
  headers = {
    'Authorization': f'Basic {base64.b64encode(basic_token).decode()}',
    'Content-Type' : 'application/x-www-form-urlencoded',
  }
  payload = {
    'grant_type':'refresh_token',
    'refresh_token': refresh_token,
  }
  response = requests.post("https://api.fitbit.com/oauth2/token", data=payload, headers=headers)
  params = json.loads(response.content)
  return params

def get_tokens():
  tokens = load_tokens()
  refresh_token = REFRESH_TOKEN if 'refresh_token' not in tokens else tokens['refresh_token']
  params = update_tokens(refresh_token)
  tokens = {
    'access_token': params['access_token'],
    'refresh_token': params['refresh_token'], 
  }
  return tokens
  
def load_tokens():
  if (os.path.exists('tokens.json') is False):
    return {}
  with open('tokens.json', 'rt') as f:
    data = json.load(f)
  return data

def save_tokens(tokens):
  with open('tokens.json', 'wt') as f:
    json.dump(tokens, f)
  

def call_api(tokens ,value):
    client = fitbit.Fitbit(CLIENT_ID, CLIENT_SECRET, tokens['access_token'], tokens['refresh_token'])
    url = "{0}/{1}/user/-/foods/log/water.json".format(*client._get_common_args())
    data = {
        'amount' : value['water_weight'],
        'unit' : 'ml',
        'date' : datetime.now().strftime('%Y-%m-%d')
    }
    client.make_request(url, data=data)


# MQTTの接続設定
client = mqtt_client.Client()          # クラスのインスタンス(実体)の作成
client.on_connect = on_connect         # 接続時のコールバック関数を登録
client.on_disconnect = on_disconnect   # 切断時のコールバックを登録
client.on_message = on_message         # メッセージ到着時のコールバック

client.connect(broker, port, 60)  # 接続先は自分自身

client.loop_forever()                  # 永久ループして待ち続ける
