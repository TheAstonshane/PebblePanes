import Image
import PIL.ImageOps
import os

s = (55,55)

for file in os.listdir(os.getcwd() + "/source"):
    img = Image.open("source/" + file)
    img = img.convert("RGB")
    img = PIL.ImageOps.invert(img) #Thanks to Michael Hosier (github.com/hosier) for this part
    img = img.convert("1")
    img = img.resize(s, Image.ANTIALIAS)
    i = 0
    if (file[1] == " "):
        file = file[2:]    
    elif (file[2] == " "):
        file = file[3:]
    elif (file[3] == " "):
        file = file[4:]
    file = file.replace(" ","_")
        
   
    img.save("resized/" + file)  
            
