BROKER_ENDPOINT = "mqttsendpoint"
TLS_PORT = 8883  # Secure port
MQTT_USERNAME = "user"  # Put here your Ubidots TOKEN
MQTT_PASSWORD = "pass"  # Leave this in blank
SUB_TOPIC = "/cs7ns6/esp32/sensors/"
PUB_TOPIC = "/cs7ns6/esp32/alerts/"
DEVICE_LABEL = "esp32-sm-01"
TLS_CERT_PATH = r"./cert"  # Put here the path of your TLS cert
SECRET = 'firebasedatabasesecret'
DSN = 'https://myapp.firebaseio.com'
EMAIL = 'name@domain'

TEMP_THRESH = 98.6
BPM_THRESH = 100

temp = 0.0
bpm = 0

authentication = FirebaseAuthentication(SECRET, EMAIL, True, True)
firebase = FirebaseApplication(DSN, authentication)


def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("[INFO] Connected to broker")
        global connected  # Use global variable
        connected = True  # Signal connection
        topic = "{}{}".format(SUB_TOPIC, DEVICE_LABEL)
        print("Subscribing to MQTT topic:", topic)
        client.subscribe(topic)
    else:
        print("[INFO] Error, connection failed")


def on_publish(client, userdata, result):
    print("Published!")


def connect(mqtt_client, mqtt_username, mqtt_password, broker_endpoint, port):
    global connected

    if not connected:
        mqtt_client.username_pw_set(mqtt_username, password=mqtt_password)
        mqtt_client.on_connect = on_connect
        mqtt_client.on_publish = on_publish
        mqtt_client.on_message = on_message
        mqtt_client.tls_set(ca_certs=None, certfile=r"./cert/cert.pem",
                            keyfile=r"./cert/privkey.pem", cert_reqs=ssl.CERT_REQUIRED,
                            tls_version=ssl.PROTOCOL_TLSv1_2, ciphers=None)
        mqtt_client.tls_insecure_set(False)
        mqtt_client.connect(broker_endpoint, port=port)
        mqtt_client.loop_start()

        attempts = 0

        while not connected and attempts < 5:  # Wait for connection
            print(connected)
            print("Attempting to connect...")
            time.sleep(1)
            attempts += 1

    if not connected:
        print("[ERROR] Could not connect to broker")
        return False

    return True


def on_message(client, userdata, msg):
    json_data = json.loads(msg.payload.decode())
    print("Received payload", json_data)
    temp = json_data['temperature']
    bpm = json_data['heartrate']
    t = time.localtime(time.time())
    current_time = time.strftime("%d-%m-%Y %H:%M:%S", t)
    trigger = compute_trigger(temp, bpm)
    payload = {'dt': current_time, 'id': DEVICE_LABEL, 'temperature': temp, 'heartrate': bpm, 'alert': trigger}
    if trigger:
        topic = "{}{}".format(PUB_TOPIC, DEVICE_LABEL)
        print("Alert!! Routing payload to topic: ",topic)
        publish(mqtt_client, topic, json.dumps(payload))
        send_mail("Possible meltdown\n", "body vitals = ", json.dumps(payload))
    firebase.post('/cs7ns6/esp32/sensors/', payload)


def publish(mqtt_client, topic, payload):
    try:
        mqtt_client.publish(topic, payload)

    except Exception as e:
        print("[ERROR] Could not publish data, error: {}".format(e))


def compute_trigger(temp, bpm):
    if temp > TEMP_THRESH or bpm > BPM_THRESH:
        return True
    return False

def _get_smtp_config(username, password):
    # Sending the email
    server = smtplib.SMTP('smtp.gmail.com', 587)
    server.ehlo()
    server.starttls()
    server.login(username, password)
    return server


def send_mail(body):
    from_address = "from@gmail.com"
    to_address = "to@tcd.ie"
    username = 'user@domain'
    password = 'pass'
    # Create message container - the correct MIME type is multipart/alternative.
    msg = MIMEMultipart('alternative')
    msg['Subject'] = "Autimeter Alert!!"
    msg['From'] = from_address
    msg['To'] = to_address
    # Create the message (HTML).
    html = body
    # Record the MIME type - text/html.
    part1 = MIMEText(html, 'html')
    # Attach parts into message container
    msg.attach(part1)
    server = _get_smtp_config(username, password)
    server.sendmail(from_address, to_address, msg.as_string())
    server.quit()

if __name__ == '__main__':
    mqtt_client = mqttClient.Client()
    connect(mqtt_client, MQTT_USERNAME, MQTT_PASSWORD, BROKER_ENDPOINT, TLS_PORT)
    mqtt_client.loop_start()
    while True:
        time.sleep(5)