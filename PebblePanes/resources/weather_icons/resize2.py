import Image
import PIL.ImageOps
import os

s = (75,55)

for file in os.listdir(os.getcwd() + "/source"):
    img = Image.open("source/" + file)
    img = img.convert("RGB")
    img = PIL.ImageOps.invert(img)
    img = img.convert("1")
    img = img.resize(s, Image.ANTIALIAS)
    img.save("resized/" + file)  
            
