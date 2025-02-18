import requests
import base64
credentials = "admin:password123"
encoded = base64.b64encode(credentials.encode()).decode()
# print(encoded)  # Use this in your client's Authorization header

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
    url = f"http://{ip_address}:80/get_status"
    headers = {
        'Accept': 'application/json',
        'Authorization': f'Basic {encoded}'
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
        'Content-Type': 'application/json',
        'Authorization': f'Basic {encoded}'
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
    ip = "192.168.0.186"  # Replace with your ESP32's IP address
    
    # # Example GET request
    get_response = make_get_request(ip)
    if get_response:
        print("GET Response from server:")
        print(get_response)

    # data = {
    #     "hours":    12,
    #     "minutes":  36,
    #     "seconds":  23,
    #     "day":      22,
    #     "month":    0,
    #     "year":     24,
    # }
    # put_response = make_put_request(ip, data)
    # if put_response:
    #     print("PUT Response from server:")
    #     print(put_response)
    # else:
    #     print("Failed to send PUT request to the server.")
    
    # # Example PUT request
    # data = {
    #     "data": [
    #         {
    #             "name": "New Kootamani",
    #             "bells": [
    #                 {
    #                     "pattern": [
    #                         {"time": 30000, "state": True},
    #                         {"time": 1000, "state": False}
    #                     ],
    #                     "repeat": 0
    #                 },
    #                 {
    #                     "pattern": [
    #                         {"time": 30000, "state": True},
    #                         {"time": 1000, "state": False}
    #                     ],
    #                     "repeat": 0
    #                 },
    #                 {
    #                     "pattern": [
    #                         {"time": 30000, "state": True},
    #                         {"time": 1000, "state": False}
    #                     ],
    #                     "repeat": 0
    #                 },
    #                 {
    #                     "pattern": [
    #                         {"time": 30000, "state": True},
    #                         {"time": 1000, "state": False}
    #                     ],
    #                     "repeat": 0
    #                 },
    #                 {
    #                     "pattern": [
    #                         {"time": 30000, "state": True},
    #                         {"time": 1000, "state": False}
    #                     ],
    #                     "repeat": 0
    #                 },
    #                 {
    #                     "pattern": [
    #                         {"time": 30000, "state": True},
    #                         {"time": 1000, "state": False}
    #                     ],
    #                     "repeat": 0
    #                 },
    #                 {
    #                     "pattern": [
    #                         {"time": 30000, "state": True},
    #                         {"time": 1000, "state": False}
    #                     ],
    #                     "repeat": 0
    #                 },
    #                 {
    #                     "pattern": [
    #                         {"time": 30000, "state": True},
    #                         {"time": 1000, "state": False}
    #                     ],
    #                     "repeat": 0
    #                 }
    #             ]
    #         },
    #         {
    #             "name": "MaranaMani",
    #             "bells": [
    #                 {
    #                     "pattern": [
    #                         {"time": 2000, "state": True},
    #                         {"time": 5000, "state": False}
    #                     ],
    #                     "repeat": 5
    #                 },
    #                 {
    #                     "pattern": [
    #                         {"time": 2000, "state": True},
    #                         {"time": 5000, "state": False}
    #                     ],
    #                     "repeat": 5
    #                 },
    #                 {
    #                     "pattern": [
    #                         {"time": 2000, "state": True},
    #                         {"time": 5000, "state": False}
    #                     ],
    #                     "repeat": 5
    #                 },
    #                 {
    #                     "pattern": [
    #                         {"time": 2000, "state": True},
    #                         {"time": 5000, "state": False}
    #                     ],
    #                     "repeat": 5
    #                 },
    #                 {
    #                     "pattern": [
    #                         {"time": 2000, "state": True},
    #                         {"time": 5000, "state": False}
    #                     ],
    #                     "repeat": 5
    #                 },
    #                 {
    #                     "pattern": [
    #                         {"time": 2000, "state": True},
    #                         {"time": 5000, "state": False}
    #                     ],
    #                     "repeat": 5
    #                 },
    #                 {
    #                     "pattern": [
    #                         {"time": 2000, "state": True},
    #                         {"time": 5000, "state": False}
    #                     ],
    #                     "repeat": 5
    #                 },
    #                 {
    #                     "pattern": [
    #                         {"time": 2000, "state": True},
    #                         {"time": 5000, "state": False}
    #                     ],
    #                     "repeat": 5
    #                 }
    #             ]
    #         },
    #         {
    #             "name": "KurishuMani",
    #             "bells": [
    #                 {
    #                     "pattern": [
    #                         {"time": 1000, "state": True},
    #                         {"time": 3000, "state": False},
    #                         {"time": 1000, "state": True},
    #                         {"time": 3000, "state": False},
    #                         {"time": 1000, "state": True},
    #                         {"time": 5000, "state": False}
    #                     ],
    #                     "repeat": 0
    #                 },
    #                 {
    #                     "pattern": [
    #                         {"time": 1000, "state": True},
    #                         {"time": 3000, "state": False},
    #                         {"time": 1000, "state": True},
    #                         {"time": 3000, "state": False},
    #                         {"time": 1000, "state": True},
    #                         {"time": 5000, "state": False}
    #                     ],
    #                     "repeat": 0
    #                 },
    #                 {
    #                     "pattern": [
    #                         {"time": 1000, "state": True},
    #                         {"time": 3000, "state": False},
    #                         {"time": 1000, "state": True},
    #                         {"time": 3000, "state": False},
    #                         {"time": 1000, "state": True},
    #                         {"time": 5000, "state": False}
    #                     ],
    #                     "repeat": 0
    #                 },
    #                 {
    #                     "pattern": [
    #                         {"time": 1000, "state": True},
    #                         {"time": 3000, "state": False},
    #                         {"time": 1000, "state": True},
    #                         {"time": 3000, "state": False},
    #                         {"time": 1000, "state": True},
    #                         {"time": 5000, "state": False}
    #                     ],
    #                     "repeat": 0
    #                 },
    #                 {
    #                     "pattern": [
    #                         {"time": 1000, "state": True},
    #                         {"time": 3000, "state": False},
    #                         {"time": 1000, "state": True},
    #                         {"time": 3000, "state": False},
    #                         {"time": 1000, "state": True},
    #                         {"time": 5000, "state": False}
    #                     ],
    #                     "repeat": 0
    #                 },
    #                 {
    #                     "pattern": [
    #                         {"time": 1000, "state": True},
    #                         {"time": 3000, "state": False},
    #                         {"time": 1000, "state": True},
    #                         {"time": 3000, "state": False},
    #                         {"time": 1000, "state": True},
    #                         {"time": 5000, "state": False}
    #                     ],
    #                     "repeat": 0
    #                 },
    #                 {
    #                     "pattern": [
    #                         {"time": 1000, "state": True},
    #                         {"time": 3000, "state": False},
    #                         {"time": 1000, "state": True},
    #                         {"time": 3000, "state": False},
    #                         {"time": 1000, "state": True},
    #                         {"time": 5000, "state": False}
    #                     ],
    #                     "repeat": 0
    #                 },
    #                 {
    #                     "pattern": [
    #                         {"time": 1000, "state": True},
    #                         {"time": 3000, "state": False},
    #                         {"time": 1000, "state": True},
    #                         {"time": 3000, "state": False},
    #                         {"time": 1000, "state": True},
    #                         {"time": 5000, "state": False}
    #                     ],
    #                     "repeat": 0
    #                 }
    #             ]
    #         },
    #         {
    #             "name": "Http Song",
    #             "bells": [
    #                 {
    #                     "pattern": [
    #                         {"time": 1000, "state": True},
    #                         {"time": 1000, "state": False},
    #                         {"time": 1000, "state": True},
    #                         {"time": 1000, "state": False},
    #                         {"time": 1000, "state": True},
    #                         {"time": 1000, "state": False}
    #                     ],
    #                     "repeat": 3
    #                 },
    #                 {
    #                     "pattern": [
    #                         {"time": 1000, "state": True},
    #                         {"time": 1000, "state": False},
    #                         {"time": 1000, "state": True},
    #                         {"time": 1000, "state": False},
    #                         {"time": 1000, "state": True},
    #                         {"time": 1000, "state": False}
    #                     ],
    #                     "repeat": 3
    #                 },
    #                 {
    #                     "pattern": [
    #                         {"time": 1000, "state": True},
    #                         {"time": 1000, "state": False},
    #                         {"time": 1000, "state": True},
    #                         {"time": 1000, "state": False},
    #                         {"time": 1000, "state": True},
    #                         {"time": 1000, "state": False}
    #                     ],
    #                     "repeat": 3
    #                 },
    #                 {
    #                     "pattern": [
    #                         {"time": 1000, "state": True},
    #                         {"time": 1000, "state": False},
    #                         {"time": 1000, "state": True},
    #                         {"time": 1000, "state": False},
    #                         {"time": 1000, "state": True},
    #                         {"time": 1000, "state": False}
    #                     ],
    #                     "repeat": 3
    #                 },
    #                 {
    #                     "pattern": [
    #                         {"time": 1000, "state": True},
    #                         {"time": 1000, "state": False},
    #                         {"time": 1000, "state": True},
    #                         {"time": 1000, "state": False},
    #                         {"time": 1000, "state": True},
    #                         {"time": 1000, "state": False}
    #                     ],
    #                     "repeat": 3
    #                 },
    #                 {
    #                     "pattern": [
    #                         {"time": 1000, "state": True},
    #                         {"time": 1000, "state": False},
    #                         {"time": 1000, "state": True},
    #                         {"time": 1000, "state": False},
    #                         {"time": 1000, "state": True},
    #                         {"time": 1000, "state": False}
    #                     ],
    #                     "repeat": 3
    #                 },
    #                 {
    #                     "pattern": [
    #                         {"time": 1000, "state": True},
    #                         {"time": 1000, "state": False},
    #                         {"time": 1000, "state": True},
    #                         {"time": 1000, "state": False},
    #                         {"time": 1000, "state": True},
    #                         {"time": 1000, "state": False}
    #                     ],
    #                     "repeat": 3
    #                 },
    #                 {
    #                     "pattern": [
    #                         {"time": 1000, "state": True},
    #                         {"time": 1000, "state": False},
    #                         {"time": 1000, "state": True},
    #                         {"time": 1000, "state": False},
    #                         {"time": 1000, "state": True},
    #                         {"time": 1000, "state": False}
    #                     ],
    #                     "repeat": 3
    #                 }
    #             ]
    #         }
    #     ]
    # }
    # put_response = make_put_request(ip, data)
    # if put_response:
    #     print("PUT Response from server:")
    #     print(put_response)
    # else:
    #     print("Failed to send PUT request to the server.")