import requests

# def make_get_request(ip_address):
#     url = f"http://{ip_address}:80/get?query1=1&query2=as"
#     try:
#         response = requests.get(url)
#         response.raise_for_status()  # Raise an error for bad status codes
#         return response.text
#     except requests.exceptions.RequestException as e:
#         print(f"An error occurred: {e}")
#         return None

def make_get_request(ip_address):
    url = f"http://{ip_address}:80/get_rtc"
    headers = {
        'Accept': 'application/json'
    }
    try:
        response = requests.get(url, headers=headers)
        response.raise_for_status()  # Raise an error for bad status codes
        print("Response Headers:", dict(response.headers))
        return response.text
    except requests.exceptions.RequestException as e:
        print(f"An error occurred: {e}")
        return None
    
def make_put_request(ip_address, data):
    url = f"http://{ip_address}:80/set_rtc"
    headers = {
        'Content-Type': 'application/json'
    }
    try:
        response = requests.put(url, json=data, headers=headers)
        response.raise_for_status()
        print("Response Headers:", dict(response.headers))
        return response.text
    except requests.exceptions.RequestException as e:
        print(f"An error occurred: {e}")
        return None

if __name__ == "__main__":
    ip = "192.168.0.197"  # Replace with your ESP32's IP address
    
    # Example GET request
    # get_response = make_get_request(ip)
    # if get_response:
    #     print("GET Response from server:")
    #     print(get_response)
    
    # # Example PUT request
    data = {
        "day": 13,
        "month": 2,
        "year": 24,
        "hours": 14,
        "minutes": 45,
        "seconds": 0
    }
    put_response = make_put_request(ip, data)
    if put_response:
        print("PUT Response from server:")
        print(put_response)
    else:
        print("Failed to send PUT request to the server.")