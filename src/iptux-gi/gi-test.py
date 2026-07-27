import gi
gi.require_version('Iptux', '1.0')
from gi.repository import Iptux

def main():
    config = Iptux.Config.new_from_fname("")
    core_thread = Iptux.Service.new(config)
    core_thread.start()
    core_thread.stop()
    print("CoreThread created successfully.")

if __name__ == "__main__":
    main()