import requests

def make_get_request(ip_address):
    url = f"http://{ip_address}:80/hello?query1=2&query2=example"
    try:
        response = requests.get(url)
        response.raise_for_status()  # Raise an error for bad status codes
        return response.text
    except requests.exceptions.RequestException as e:
        print(f"An error occurred: {e}")
        return None

if __name__ == "__main__":
    ip = "192.168.0.193"  # Replace with the desired IP address
    response = make_get_request(ip)
    if response:
        print("Response from server:")
        print(response)
    else:
        print("Failed to get a response from the server.")