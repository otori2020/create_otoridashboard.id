import datetime
import jwt
import ssl
import time

import paho.mqtt.client as mqtt

#Project ID of IoT Core
PROJECT_ID = "lab-12-05-2020"
# Location of server
REGION_ID = "asia-east1"
# ID of IoT registry
REGISTRY_ID = "your_registry"
# ID of this device
DEVICE_ID = "your_device_name"

# Type of encryption being used
ENCRYPTION_ALGORITHM = "ES256"

# Private Key fike
PRIVATE_KEY_FILE = "C:/Python/ec_private.pem"

# Certificate for Google SSL
CA_CERTS = "C:/Python/roots.pem"

# Lifetime of credentials to send message
JWT_EXPIRES_IN_MINUTES = 10

# Google IoT MQTT Bridge
MQTT_BRIDGE_HOSTNAME = "mqtt.googleapis.com"
MQTT_BRIDGE_PORT = 8883

# Timeout to wait for connection
WAIT_CONNECTION_TIMEOUT = 5

# Connection status
connected = False


def create_jwt():
    """ Create JWT Token """
    iat = datetime.datetime.utcnow()
    exp = iat + datetime.timedelta(minutes=JWT_EXPIRES_IN_MINUTES)
    print("iat", iat)
    print("exp", exp)

    token = {
        # The time the token was issued.
        'iat': iat,
        # Token expiration time.
        'exp': exp,
        # The audience field should always be set to the GCP project id.
        'aud': PROJECT_ID
    }
    print("token", token)

    # Read the private key file.
    pem_file = open(PRIVATE_KEY_FILE, 'r')
    private_key = pem_file.read()

    print(f"Creating JWT using '{ENCRYPTION_ALGORITHM}' from private key file '{PRIVATE_KEY_FILE}'.")

    jwt_token = jwt.encode(token, private_key, algorithm=ENCRYPTION_ALGORITHM).encode('ascii')
    print()
    print("JWT TOKEN")
    print(jwt_token)
    print()

    return jwt_token


def error_str(rc):
    """Convert a Paho error to a human readable string."""
    return f"{rc}: {mqtt.error_string(rc)}"


def on_connect(unused_client, unused_userdata, unused_flags, rc):
    """Callback for when a device connects."""
    #print('on_connect: ', mqtt.connack_string(rc))
    print(f"on_connect: {error_str(rc)} ({mqtt.connack_string(rc)})")
    print()

    global connected
    connected = True


def on_disconnect(unused_client, unused_userdata, rc):
    """Paho callback for when a device disconnects."""
    print(f"on_disconnect: {error_str(rc)}")
    print()

    global connected
    connected = False


def on_publish(client, userdata, mid):
    """Paho callback when a message is sent to the broker."""
    print('on_publish')
    print("userdata:" + str(userdata))
    print("mid:" + str(mid))
    print()


def on_subscribe(client, userdata, mid, granted_qos):
    print("on_subscribe")
    print()


def on_unsubscribe(client, userdata, mid):
    print("on_unsubscribe")
    print()


def on_message(client, userdata, message):
    """Callback when the device receives a message on a subscription."""
    payload = str(message.payload.decode('utf-8'))
    print(f"Received message \'{payload}\' on topic \'{message.topic}\' with Qos {str(message.qos)}")
    print()


def wait_for_connection(timeout):
    """Wait for the device to become connected."""
    global connected 

    total_time = 0
    while not connected and total_time < timeout:
        time.sleep(1)
        total_time += 1

    if not connected:
        raise RuntimeError('Could not connect to MQTT bridge.')


def get_client():
    # create client Object
    client_id = f"projects/{PROJECT_ID}/locations/{REGION_ID}/registries/{REGISTRY_ID}/devices/{DEVICE_ID}"
    client = mqtt.Client(client_id=client_id)

    # With Google Cloud IoT Core, the username field is ignored, and the
    # password field is used to transmit a JWT to authorize the device.
    client.username_pw_set(username='unused',
                           password=create_jwt())

    # Use SSL/TLS support
    client.tls_set(ca_certs=CA_CERTS, tls_version=ssl.PROTOCOL_TLSv1_2)

    # Register message callbacks. https://eclipse.org/paho/clients/python/docs/
    # describes additional callbacks that Paho supports. In this example, the
    # callbacks just print to standard out.
    client.on_connect = on_connect
    client.on_disconnect = on_disconnect
    client.on_publish = on_publish
    client.on_subscribe = on_subscribe
    client.on_unsubscribe = on_unsubscribe
    client.on_message = on_message

    # Connect to the Google MQTT bridge.
    client.connect(MQTT_BRIDGE_HOSTNAME, MQTT_BRIDGE_PORT)
    client.loop_start()
    wait_for_connection(WAIT_CONNECTION_TIMEOUT)

    return client


def wait_for_disconnection(timeout):
    """Wait for the device to become connected."""
    global connected 

    total_time = 0
    while connected and total_time < timeout:
        time.sleep(1)
        total_time += 1

    if connected:
        raise RuntimeError('Could not disconnect to MQTT bridge.')


def release_client(client):
    """"Disconnect device from broker."""
    client.disconnect()
    client.loop_stop()
    wait_for_disconnection(WAIT_CONNECTION_TIMEOUT)


def publish_events():
    """Publish an event."""
    print()
    print("Publish Events")
    print("================================================")
    print()

    client = get_client()

    # Publish to the events
    mqtt_topic = f"/devices/{DEVICE_ID}/events"

    # Create payload
    payload = "Event: " + str(datetime.datetime.now())

    # Publish something
    print("Publishing")
    print(mqtt_topic)
    print(payload)
    print()
    # Publish "payload" to the MQTT topic. qos=1 means at least once
    # delivery. Cloud IoT Core also supports qos=0 for at most once
    # delivery.
    message = client.publish(mqtt_topic, payload, qos=1)    
    message.wait_for_publish()
  
    release_client(client)


def publish_state():
    """Publish a state."""
    print()
    print("Publish State")
    print("================================================")
    print()

    client = get_client()

    # Publish to the events
    mqtt_topic = f"/devices/{DEVICE_ID}/state"

    # Create payload
    payload = "State: " + str(datetime.datetime.now())

    # Publish something
    print("Publishing")
    print(mqtt_topic)
    print(payload)
    print()
    # Publish "payload" to the MQTT topic. qos=1 means at least once
    # delivery. Cloud IoT Core also supports qos=0 for at most once
    # delivery.
    message = client.publish(mqtt_topic, payload, qos=1)    
    message.wait_for_publish()
  
    release_client(client)


def subscribe_config():
    """Subscribe to new configurations.

    New configurations. are sent to this topic. Device need to remember if it has the latest version or not since topic will keep being sent.
    """

    print()
    print("Subscribe Config")
    print("================================================")
    print()

    client = get_client()

    # Subscribe to the events
    mqtt_topic = f"/devices/{DEVICE_ID}/config"

    # Subscribe to the config topic.
    print("Subscribing")
    print(mqtt_topic)
    print()
    client.subscribe(mqtt_topic, qos=1)
    time.sleep(5)

    # Unsubscribe to the config topic.
    print("Unsubscribing")
    print()
    client.unsubscribe(mqtt_topic)
    time.sleep(5)

    release_client(client)


def subscribe_command():
    """Subscribe to commands from controller."""
    print()
    print("Subscribe Command")
    print("================================================")
    print()

    client = get_client()

    # Subscribe to the events
    mqtt_topic = f"/devices/{DEVICE_ID}/commands/#"

    # Subscribe to the config topic.
    print("Subscribing")
    print(mqtt_topic)
    print()
    client.subscribe(mqtt_topic, qos=1)
    time.sleep(5)

    # Unsubscribe to the config topic.
    print("Unsubscribing")
    print()
    client.unsubscribe(mqtt_topic)
    time.sleep(5)

    release_client(client)


publish_events()
#publish_state()
#subscribe_config()
#subscribe_command()
