import pefile
import sys

def list_dll_exports(dll_path):
    try:
        pe = pefile.PE(dll_path)
        
        if hasattr(pe, 'DIRECTORY_ENTRY_EXPORT'):
            for exp in pe.DIRECTORY_ENTRY_EXPORT.symbols:
                if exp.name:
                    export_name = exp.name.decode('utf-8')
                    if "stop" in export_name or "update" in export_name or "setPaused" in export_name or "setVolume" in export_name:
                        print(f"[found] -> {export_name}")
                    else:
                        # print(export_name)
                        pass
        else:
            print("no export table.")
            
    except Exception as e:
        print(f"err: {e}")

if __name__ == "__main__":
    list_dll_exports("fmod.dll")