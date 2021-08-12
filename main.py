from flask import Flask, request, redirect, render_template, url_for

from processor import *


app = Flask(__name__)
processor = Processor()


@app.route('/loadImage', methods = ['GET', 'POST'])
def loadImage():
    if request.method == 'POST':
        file = request.files['file']
        return processor.load(file)
    return render_template('load_image.html')


@app.route('/filters', methods = ['GET', 'POST'])
def filters():
    if processor.inputImage['empty'] == True:
        return redirect(url_for('loadImage'))

    if request.method == 'POST':
        params = request.form
        operation = None
        cont = 0
        for i in processor.switcher.keys():
            if cont == int(params['operation']):
                operation = i
                break
            cont += 1
        return processor.process(operation, params)

    if request.method == 'GET':
        imgBase64 = CVImageToBase64(processor.inputImage['img'])
        print(imgBase64[:80])
        operatorsList = processor.getOpList()
        print(operatorsList)
        page_corners = processor.getPoints()
        return render_template(
            'filters.html',
            img = imgBase64,
            opList = operatorsList,
            top_left = page_corners[0], top_right = page_corners[1],
            bot_left = page_corners[2], bot_right = page_corners[3]
        )


@app.route('/scan', methods = ['GET', 'POST'])
def scan():
    if processor.outputImage['empty'] == True:
        return redirect(url_for('loadImage'))

    if request.method == 'POST':
        return processor.download()

    imgBase64 = CVImageToBase64(processor.outputImage['img'])
    print(imgBase64[:80])

    return render_template('scan.html', img = imgBase64)


@app.route('/process', methods = ['GET', 'POST'])
def process():
    if request.method == 'POST':
        file = request.files['file']
        return processor.process(file, request.form)
    return render_template('scanner_form.html')


@app.route('/download', methods = ['GET'])
def download():
    if processor.inputImage['empty'] == True:
        return redirect(url_for('loadImage'))
    return processor.download()


@app.route('/', methods = ['GET'])
def base():
    return redirect(url_for('loadImage'))


if __name__ == '__main__':
    app.secret_key = 'some secret key'
    compileFlag = input('Do you want to compile C++ librarys? (Y/N): ')
    if compileFlag == 'Y' or compileFlag == 'y':
        cleanAfterBuildFlag = input('Do you want to clean after build? (Y/N): ')
        processor.compileLibrary(cleanAfterBuildFlag)
    app.run('0.0.0.0')
