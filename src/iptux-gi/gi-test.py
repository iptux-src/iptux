import gi
gi.require_version('Iptux', '1.0')
from gi.repository import Iptux
import time

def main():
    config = Iptux.Config.new_from_fname("")
    core_thread = Iptux.Service.new(config)
    core_thread.start()

    time.sleep(2)  # Wait for the core thread to initialize

    print(dir(core_thread))  # List available methods and attributes
    pals = core_thread.get_pals()
    for x in pals:
        print(f"Pal Info: {x.to_string()}")  # Assuming to_string() method exists


    core_thread.send_message(pals[0], "Hello from Python!")  # Send a message to the first pal

    core_thread.stop()
    print("CoreThread created successfully.")

if __name__ == "__main__":
    main()
