import cv2
import matplotlib.pyplot as plt
import cvlib as cv
import numpy as np
from cvlib.object_detection import draw_bbox
import pyrebase


config = {

  "apiKey": "AIzaSyAB1ndzvcM83yea382JtjpwneGSDrc1U_A",

  "authDomain": "smartintrudersystem.firebaseapp.com",

  "databaseURL": "https://smartintrudersystem-default-rtdb.firebaseio.com",

  "projectId": "smartintrudersystem",

  "storageBucket": "smartintrudersystem.appspot.com",

  "messagingSenderId": "55076380360",

  "appId": "1:55076380360:web:84edf20470478c8b294ffd",

  "measurementId": "G-G4JF7Y1153"


};

firebase = pyrebase.initialize_app(config)
storage = firebase.storage()
db = firebase.database()

storage_loc = 'data/photo.jpg'
new_storage_loc = 'data/test-image.jpg'
file_name = 'test.jpg'

storage.child(storage_loc).download(storage_loc,file_name)
im = cv2.imread(file_name)
#cv2.namedWindow("live transmission", cv2.WINDOW_AUTOSIZE)
cv2.imshow('Original transmission',im)

bbox, label, conf = cv.detect_common_objects(im)
image = draw_bbox(im, bbox, label, conf)
for x in label:
    if x == "person":
        #print("Hello ")
#cv2.namedWindow("Detected transmission", cv2.WINDOW_AUTOSIZE)
        cv2.imshow('detection',image)
        status = cv2.imwrite(r'C:\Users\psama\AppData\Local\Programs\Python\Python311\GroupProject Open CV\test1.jpeg',image)  
#print("Image written to file-system : ", status)  
        file_new_path = r'C:\Users\psama\AppData\Local\Programs\Python\Python311\GroupProject Open CV\test1.jpeg'
        storage.child(new_storage_loc).put(file_new_path)

        auth=firebase.auth();
        email = "smartintrudersystem@gmail.com"
        password = "Smart@1234"

        user = auth.sign_in_with_email_and_password(email,password)
        url = storage.child(new_storage_loc).get_url(user['idToken'])
        print(url)

        data={"image":url}
        db.update(data)



