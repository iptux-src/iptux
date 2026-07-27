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
        print(f"Pal Info: {repr(x)}")

    core_thread.stop()
    print("CoreThread created successfully.")

if __name__ == "__main__":
    main()
