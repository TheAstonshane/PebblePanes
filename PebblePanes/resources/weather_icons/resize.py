import Image
import PIL.ImageOps

f_ext = ".png"
size = 60
s = (size,size)
dir1 = "source/"
dir2 = "resized/"


img = Image.open(dir1 + "001lighticons-02.png")
img = img.convert("RGB")
img = PIL.ImageOps.invert(img)
img = img.convert("1")
img = img.resize(s, Image.ANTIALIAS)
img.save(dir2+"sun" + f_ext)


img = Image.open(dir1 +"001lighticons-03.png")
img = img.convert("RGB")
img = PIL.ImageOps.invert(img)
img = img.convert("1")
img = img.resize(s, Image.ANTIALIAS)
img.save(dir2+"moon" + f_ext)


img = Image.open(dir1 +"001lighticons-06.png")
img = img.convert("RGB")
img = PIL.ImageOps.invert(img)
img = img.convert("1")
img = img.resize(s, Image.ANTIALIAS)
img.save(dir2+"wind" + f_ext)

img = Image.open(dir1 +"001lighticons-08.png")
img = img.convert("RGB")
img = PIL.ImageOps.invert(img)
img = img.convert("1")
img = img.resize(s, Image.ANTIALIAS)
img.save(dir2+"mostly_cloudy_day"+ f_ext)

img = Image.open(dir1 +"001lighticons-09.png")
img = img.convert("RGB")
img = PIL.ImageOps.invert(img)
img = img.convert("1")
img = img.resize(s, Image.ANTIALIAS)
img.save(dir2+"mostly_cloudy_night"+ f_ext)

img = Image.open(dir1 +"001lighticons-14.png")
img = img.convert("RGB")
img = PIL.ImageOps.invert(img)
img = img.convert("1")
img = img.resize(s, Image.ANTIALIAS)
img.save(dir2+"cloudy"+ f_ext)

img = Image.open(dir1 +"001lighticons-15.png")
img = img.convert("RGB")
img = PIL.ImageOps.invert(img)
img = img.convert("1")
img = img.resize(s, Image.ANTIALIAS)
img.save(dir2+"lightning"+ f_ext)

img = Image.open(dir1 +"001lighticons-17.png")
img = img.convert("RGB")
img = PIL.ImageOps.invert(img)
img = img.convert("1")
img = img.resize(s, Image.ANTIALIAS)
img.save(dir2+"light_rain"+ f_ext)

img = Image.open(dir1 +"001lighticons-18.png")
img = img.convert("RGB")
img = PIL.ImageOps.invert(img)
img = img.convert("1")
img = img.resize(s, Image.ANTIALIAS)
img.save(dir2+"heavy_rain"+ f_ext)

img = Image.open(dir1 +"001lighticons-21.png")
img = img.convert("RGB")
img = PIL.ImageOps.invert(img)
img = img.convert("1")
img = img.resize(s, Image.ANTIALIAS)
img.save(dir2+"light_snow" + f_ext)

img = Image.open(dir1 +"001lighticons-23.png")
img = img.convert("RGB")
img = PIL.ImageOps.invert(img)
img = img.convert("1")
img = img.resize(s, Image.ANTIALIAS)
img.save(dir2+"heavy_snow"+ f_ext)