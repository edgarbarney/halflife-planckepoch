rem Put a copy command in here to copy your client.dll into your mod directory
robocopy ".\projects\vs2019\Debug\hl_cdll" "F:\SteamLibrary\steamapps\common\Half-Life\SpiritTrinity\cl_dlls" client.dll /njh /njs /ndl /nc /ns /np
robocopy ".\projects\vs2019\Debug\hl_cdll" "F:\SteamLibrary\steamapps\common\Half-Life\SpiritTrinity\cl_dlls" client.pdb /njh /njs /ndl /nc /ns /np
