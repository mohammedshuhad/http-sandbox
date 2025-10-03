import requests
import base64
import json


credentials = "admin:password123"
encoded = base64.b64encode(credentials.encode()).decode()


def make_get_request_with_dom_query(ip_address, endpoint, query_param):
    url = f"http://{ip_address}:80/{endpoint}?dom_index={query_param}"
    headers = {
        'Accept': 'application/json',
        'Authorization': f'Basic {encoded}'
    }
    try:
        response = requests.get(url, headers=headers)
        response.raise_for_status()  # Raise an error for bad status codes
        return response.text
    except requests.exceptions.RequestException as e:
        print(f"An error occurred: {e}")
        return None

def make_get_request(ip_address, endpoint):

    url = f"http://{ip_address}:80/{endpoint}"
    headers = {
        'Accept': 'application/json',
        'Authorization': f'Basic {encoded}'
    }
    try:
        response = requests.get(url, headers=headers)
        response.raise_for_status()  
        print("Response Headers:", dict(response.headers))
        return response.text
    except requests.exceptions.RequestException as e:
        print(f"An error occurred: {e}")
        return None

def get_ring_bell(ip_address):
    endpoint = "get_ring_bell"
    bell_info = make_get_request(ip_address, endpoint)
    return bell_info

def get_repeat_bell(ip_address):
    endpoint = "get_repeat_bell"
    repeat_bell_info = make_get_request(ip_address, endpoint)
    return repeat_bell_info

def view_bells(ip_address):
    endpoint = "view_bells"
    view_info = make_get_request(ip_address, endpoint)
    return view_info

def get_all_dom(ip_address):
    endpoint = "get_all_dom"
    all_dom_info = make_get_request(ip_address,endpoint)
    return all_dom_info

def get_dom(ip_address, dom_index):
    endpoint = "get_dom"
    dom_info = make_get_request_with_dom_query(ip_address, endpoint, dom_index)
    return dom_info

def dom_view_bell(ip_address):
    endpoint = "dom_view_bell"
    dom_index = 0
    dom_info = make_get_request_with_dom_query(ip_address, endpoint, dom_index)
    return dom_info

def get_dop(ip_address):
    endpoint = "get_dop"
    dop_info = make_get_request(ip_address, endpoint)
    return dop_info

def dop_view_bell(ip_address):
    endpoint = "dop_view_bell"
    dop_info = make_get_request(ip_address, endpoint)
    return dop_info

####                                              SET                                                  ####

def make_set_request_with_dom_query(ip_address, endpoint, query_param, data):
    url = f"http://{ip_address}:80/{endpoint}?dom_index={query_param}"
    headers = {
        'Accept': 'application/json',
        'Authorization': f'Basic {encoded}'
    }
    try:
        response = requests.put(url, json=data, headers=headers)
        response.raise_for_status()  # Raise an error for bad status codes
        return response.text
    except requests.exceptions.RequestException as e:
        print(f"An error occurred: {e}")
        return None
    
    
def make_put_request(ip_address, endpoint, data):
    url = f"http://{ip_address}:80/{endpoint}"
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
    except ValueError as e:
        print(f"Invalid JSON response: {e}")
        return None

def set_season(ip_address, season_index):
    data = {
        "season_index": season_index
    }
    return make_put_request(ip_address, "set_season", data)

def set_auto_sleep(ip_address, value):
    if value not in(True, False):
        print("Enter True or False")
        return None

    data = {
        "auto_sleep": value
    }
    return make_put_request(ip_address, "set_auto_sleep", data)

def set_auto_bell(ip_address, value):
    if value not in(True, False):
        print("Enter True or False")
        return None

    data = {
        "auto_bell": value
    }
    return make_put_request(ip_address, "set_auto_bell", data)

def set_select_bell(ip_address, bell_index, state):
    if bell_index not in (1, 2, 3, 4, 5, 6, 7, 8):
        print("Enter valid bell index ")
        return None
    
    if not isinstance(state, bool):
        print("State must be True or False")
        return None
    
    data = {
        "select_bell": bell_index,
        "all_bell":state
    }
    return make_put_request(ip_address, "set_select_bell", data)

def set_ring_bell(ip_address, bell_index, ring, stop):
    if bell_index not in (0, 1, 2, 3):
        print("Enter valid bell index")
        return None
    if not isinstance(ring, bool):
        print("Ring now must be True or False")
        return None
    
    if not isinstance(stop, bool):
        print("Stop now must be True or False")
        return None
    
    if ring == stop:
        print("Ring now and Stop must be different")
        return None
    
    data = {
        "bell_index": bell_index,
        "ring_now": ring,
        "stop_ring": stop
        
    }
    return make_put_request(ip_address, "set_ring_bell", data)

def set_repeat_bell(ip_address, repeat, interval, up_to_time, bell_index ):
    if not isinstance(repeat, bool):
        print("Repeat must be True or False")
        return None
    if interval < 1 or interval > 60:
        print("Interval must be in (1-60)")
        return None
    if up_to_time < 1 or up_to_time > 100:
        print("Up_to_time must be in (1-100)")
        return None
    if bell_index < 0 or bell_index > 3:
        print("Bell index must be in (0-3)")
        return None
    data = {
        "repeat" : repeat,
        "interval": interval,
        "up_to_time": up_to_time,
        "bell_index": bell_index
    }
    return make_put_request(ip_address,"set_repeat_bell", data)

def add_bell(ip_address, bell_index_arr, repeat_count_arr, season_index, hour, minute, schedule, dom_day, dom_occurence, dop_day, dop_month, dop_year, schedule_type):
    if bell_index < 0 or bell_index > 3:
        print("Bell index must be in (0-3)")
        return None
    if repeat_count_arr < 0 or repeat_count_arr > 3:
        print("Repeat index must be in (0-3)")
        return None
    if season_index < 0 or season_index > 4:
        print("Invalid season !!\n Enter SUMMER = 0, WINTER = 1,CHRISTMAS = 2,EASTER = 3,EXAM = 4,")
        return None 
    if hour < 0 or hour > 23:
        print("Enter valid hour")
        return None
    if minute < 0 or minute > 59:
        print("Enter valid minute")
        return None
    if not isinstance(schedule, list) or not all(isinstance(i, bool) for i in schedule):
        print("Invalid schedule format. It should be a list of booleans.")
        return None
    if dom_day < 0 or dom_day > 6:
        print("Enter valid dom day")
        return None
    if dom_occurence < 1 or dom_occurence >5:
        print("Enter valid dom occurence")
        return None
    if dop_day < 0 or dop_day >1:
        print("Enter valid dop day")
        return None
    if dop_month < 1 or dop_month > 12:
        print("Enter valid dop month")
        return None
    if dop_year < 2000 or dop_month >2100:
        print("Enter valid dop year")
        return None
    if schedule_type < 0 or schedule_type > 2:
        print("Enter valid schedule type ")
        return None
    
    data = {
        "bell_index_arr": bell_index_arr,
        "repeat_count" : repeat_count_arr,
        "season_index": season_index,
        "hour": hour,
        "min": minute,
        "schedule": schedule,
        "dom_day" : dom_day,
        "dom_occurence" : dom_occurence,
        "dop_day" : dop_day,
        "dop_month" : dop_month,
        "dop_year" : dop_year,
        "schedule_type" : schedule_type
    }
    return make_put_request(ip_address, "add_bell", data)

def edit_bell(ip_address, bell_id, bell_index, season_index, hour, minute, schedule):
    data = {
        "bell_id": bell_id,
        "bell_index": bell_index,
        "season_index": season_index,
        "hour": hour,
        "min": minute,
        "schedule": schedule
    }
    return make_put_request(ip_address, "edit_bell", data)

def view_bells_with_query(ip_address, query):

    url = f"http://{ip_address}/view_bells?type={query}"
    headers = {
        'Accept': 'application/json',
        'Authorization': f'Basic {encoded}'
    }
    try:
        response = requests.get(url, headers = headers)
        response.raise_for_status()  
        return response.text
    except requests.RequestException as e:
        print("Error:", e)
        return None

def delete_bell(ip_address, bell_id):
    data = {
        "bell_id": bell_id
    }
    return make_put_request(ip_address, "delete_bell", data)

def set_all_dom(ip_address):
    data = {
        "data": [
            {
                "dom_index": 0,
                "dom_name": "Summer solicitice",
                "activate": True,
                "day": 0,
                "occurrence": 4
            },
            {
                "dom_index": 1,
                "dom_name": "summer 2",
                "activate": False,
                "day": 0,
                "occurrence": 4
            },
            {
                "dom_index": 2,
                "dom_name": "Summer 3",
                "activate": True,
                "day": 0,
                "occurrence": 4
            },
            {
                "dom_index": 3,
                "dom_name": "Winter SOlicitice",
                "activate": True,
                "day": 0,
                "occurrence": 4
            },
            {
                "dom_index": 4,
                "dom_name": "Day of Month 2",
                "activate": True,
                "day": 0,
                "occurrence": 4
            },
            {
                "dom_index": 5,
                "dom_name": "Day of Month 3",
                "activate": False,
                "day": 0,
                "occurrence": 4
            }
        ]
    }
    return make_put_request(ip_address, "set_all_dom", data)

def set_dom(ip_address, dom_index, day, occurence):
    endpoint = "set_dom"
    data = { 
        "activate": True, 
        "day": day, 
        "occurrence": occurence 
    }
    dom_info = make_set_request_with_dom_query(ip_address, endpoint, dom_index, data)
    return dom_info

def dom_add_bell(ip_address, bell_index, hour, minute):
    data = {
        "bell_index": bell_index,
        "hour": hour,
        "minute": minute
    }
    return make_put_request(ip_address, "dom_add_bell", data)

def dom_edit_bell(ip_address, bell_id, bell_index, hour, minute):
    data = {
        "bell_id": bell_id,
        "bell_index": bell_index,
        "hour": hour,
        "minute": minute
    }
    return make_put_request(ip_address, "dom_edit_bell", data)

def dom_delete_bell(ip_address, bell_id):
    data = {
        "bell_id": bell_id
    }
    return make_put_request(ip_address, "dom_delete_bell", data)

def set_dop(ip_address, dop_name, activate, day, month, year):
    data = {
        "dop_name": dop_name,
        "activate": activate,
        "day": day,
        "month": month,
        "year": year
    }
    return make_put_request(ip_address, "set_dop", data)

def dop_add_bell(ip_address, bell_index, hour, minute):
    data = {
        "bell_index": bell_index,
        "hour": hour,
        "minute": minute
    }
    return make_put_request(ip_address, "dop_add_bell", data)

def dop_edit_bell(ip_address, bell_id, bell_index, hour, minute):
    data = {
        "bell_id": bell_id,
        "bell_index": bell_index,
        "hour": hour,
        "minute": minute
    }
    return make_put_request(ip_address, "dop_edit_bell", data)

def dop_delete_bell(ip_address, bell_id):
    data = {
        "bell_id": bell_id
    }
    return make_put_request(ip_address, "dop_delete_bell", data)

def send_firmware(ip_address, firmware_path):
    url = f"http://{ip_address}:80/set_ota_bin"

    print(f"Uploading firmware to {url}...")
    
    # Basic auth credentials
    headers = {
        'Authorization': f'Basic {credentials}',
        'Content-Type': 'application/octet-stream'
    }
    
    try:
        # Open and read the firmware file in binary mode
        with open(firmware_path, 'rb') as f:
            firmware_data = f.read()
        
        # Send PUT request
        response = requests.put(
            url, 
            data=firmware_data,
            headers=headers,
            timeout=30  # Increased timeout for large files
        )
        
        # Check response
        if response.status_code == 200:
            print("Firmware upload successful!")
            print("Response:", response.json())
        else:
            print(f"Error: Server returned status code {response.status_code}")
            print("Response:", response.text)
            
    except Exception as e:
        print(f"Error occurred: {str(e)}")

def load_bell_patterns(json_file_path):
    try:
        with open(json_file_path, 'r') as file:
            return json.load(file)
    except FileNotFoundError:
        print(f"Error: Could not find {json_file_path}")
        return None
    except json.JSONDecodeError:
        print(f"Error: Invalid JSON format in {json_file_path}")
        return None

def set_bell_patterns(ip_address):
    patterns = load_bell_patterns('bellpattern.json')
    if patterns:
        return make_put_request(ip_address, "set_patterns", patterns)
    return None

def test_all_get(ip):
    methods = [get_ring_bell, get_repeat_bell, view_bells, get_all_dom, dom_view_bell, get_dop, dop_view_bell]
    failed_methods = []

    for method in methods:
        response = method(ip)
        if response:
            print("PUT Response from server (Set):")
            print(response)
        else:
            print("Failed to set")
            failed_methods.append(method.__name__)

    if failed_methods:
        print("The following methods failed:", failed_methods)
    else:
        print("All methods succeeded.")

def test_all_put(ip):
    test_cases = [
        (set_season, [ip, 0]),
        (set_auto_sleep, [ip, False]),
        (set_auto_bell, [ip, False]),
        (set_select_bell, [ip, 8, True]),
        (set_ring_bell, [ip, 0, True, False]),
        (set_repeat_bell, [ip, False, 1, 1, 3]),
        (add_bell, [ip, 3, 0, 12, 0, [False,True,False,False,False,False,True]]),
        (edit_bell, [ip, 2, 2, 1, 8, 40, [True, True, False, False, False, True, False]]),
        (delete_bell, [ip, 2]),
        (set_dop, [ip, "Day of Program", False, 25, 3, 35]),
        (dop_add_bell, [ip, 3, 13, 50]),
        (dop_edit_bell, [ip, 0, 0, 4, 55]),
        (dop_delete_bell, [ip, 3]),
        (dom_add_bell, [ip, 3, 11, 16]),
        (dom_edit_bell, [ip, 3, 3, 0, 0]),
        (dom_delete_bell, [ip, 1]),
        (set_all_dom, [ip]),
        (set_bell_patterns, [ip])
    ]

    results = {
        'success': [],
        'failed': []
    }

    for func, args in test_cases:
        response = func(*args)
        if response:
            results['success'].append(func.__name__)
            print(f"✓ {func.__name__} succeeded")
        else:
            results['failed'].append(func.__name__)
            print(f"✗ {func.__name__} failed")

    # Print summary
    print("\nTest Summary:")
    print(f"Successful calls: {len(results['success'])}")
    print(f"Failed calls: {len(results['failed'])}")
    
    if results['failed']:
        print("\nFailed calls details:")
        for func_name in results['failed']:
            print(f"- {func_name}")
            
    return results

if __name__ == "__main__":

    ip = "192.168.29.22"  # Replace with your ESP32's IP address


    # test_all_get(ip)
    # test_all_put(ip)
    
    # get_response = get_ring_bell(ip)
    # get_response = get_repeat_bell(ip)
    # get_response = get_dop(ip)
    # get_response = view_bells(ip)


    get_response = view_bells_with_query(ip, 1)


    # get_response = dop_view_bell(ip)
    # get_response = dom_view_bell(ip)
    # get_response = get_all_dom(ip)

    # if get_response:
    #     print(get_response)
    # else:
    #     print(f"Failed to get for dom_index")


    # put_response = set_season(ip, 1)
    # put_response = set_auto_sleep(ip, False)
    # put_response = set_auto_bell(ip, False)
    # put_response = set_select_bell(ip, 8, True)
    # put_response = set_ring_bell(ip, 0, True, False)
    # put_response = set_repeat_bell(ip, False, 1, 1, 3)
    # put_response = add_bell(ip, [3, 1, 1], [1, 0, 0], 0, 13, 0, [False,True,False,False,False,False,True], 2, 2, 12, 9, 25, 0)
    # put_response = send_firmware(ip, "pulsator-firmware.bin")
    # put_response = edit_bell(ip, 2, 2, 1, 8, 40, [True, True, False, False, False, True, False])
    # put_response = delete_bell(ip, 2)
    # put_response = set_dop(ip, "Day of Program", False, 25, 3, 35)
    # put_response = dop_add_bell(ip, 3, 13, 50)
    # put_response = dop_edit_bell(ip, 0, 0, 4, 55)
    # put_response = dop_delete_bell(ip, 3)
    # put_response = dom_add_bell(ip, 3, 11, 16)
    # put_response = dom_edit_bell(ip, 3, 3, 0, 0)
    # put_response = dom_delete_bell(ip, 1)
    # put_response = set_all_dom(ip)
    # put_response = set_bell_patterns(ip)

    # if put_response:
    #     print("PUT Response from server (Set):")
    #     print(put_response)
    # else:
    #     print("Failed to set")


    if get_response:
        print("GET response from the client(Get):")
        print(get_response)
    else:
        print("Failed to get")