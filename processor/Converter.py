import io
import cv2
import base64
import numpy as np
from flask import send_file


def fileToCVImage(file, CV_COLORTYPE = cv2.IMREAD_GRAYSCALE):
    imageStream = io.BytesIO(file.stream.read())
    imageStream.seek(0)
    fileBytes = np.asarray(bytearray(imageStream.read()), dtype = np.uint8)
    img = cv2.imdecode(fileBytes, CV_COLORTYPE)
    return img


def CVImageToFile(img):
    _, buffer = cv2.imencode('.jpg', img)
    bytes = io.BytesIO(buffer.tobytes())
    return bytes


def CVImageToBase64(img):
    imgStr = cv2.imencode('.jpg', img)[1].tobytes()
    npArr = np.frombuffer(imgStr, np.uint8)
    imgBase64 = base64.encodebytes(npArr).decode('ascii')
    return imgBase64


def sendImage(img, filename):
    return send_file(
        CVImageToFile(img),
        mimetype = 'image/jpg',
        as_attachment = True,
        attachment_filename = filename
    )
