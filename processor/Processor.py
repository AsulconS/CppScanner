import os
from flask.helpers import url_for
from werkzeug.utils import redirect

from .Utils import *
from .Converter import *
from library import *

class Processor:
    def __init__(self):
        self.inputImage   = { 'empty' : True }
        self.outputImage  = { 'empty' : True }
        self.croppedImage = { 'empty' : True }
        self.switcher = {}

        self.switcher['Corner Detection'] = self.opCornerDetection
        self.switcher['Process Image'] = self.opProcessImage

        # points (img) -> [coordinates]
        # scan (img, [coordinates]) -> img
        ######################################################################


    def cleanObjectFiles(self):
        directory = '.'
        filesInDirectory = os.listdir(directory)
        filteredFiles = [file for file in filesInDirectory if file.endswith('.o')]
        for file in filteredFiles:
            pathToFile = os.path.join(directory, file)
            os.remove(pathToFile)


    def compileLibrary(self, cleanAfterBuildFlag):
        print("Compiling.....")
        cleanParam = '--clean-first' if cleanAfterBuildFlag == 'Y' or cleanAfterBuildFlag == 'y' else ''
        cwd = os.getcwd().replace('\\', '/')
        cbd = cwd + '/build'
        genArgs = '-G \"Visual Studio 16 2019\" -T host=x86 -A win32'
        genCmd = f'cmake --no-warn-unused-cli -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE \"-H{cwd}\" \"-B{cbd}\" {genArgs}'
        buildCmd = f'cmake --build \"{cbd}\" {cleanParam} --config Release --target ALL_BUILD -j 10 --'
        os.system(genCmd)
        os.system(buildCmd)


    def load(self, file):
        self.inputImage['empty'] = False
        img = fileToCVImage(file, cv2.IMREAD_COLOR)
        self.inputImage['img'] = img
        self.inputImage['name'] = file.filename.replace(' ', '_')[:-4] + '.png'
        return redirect(url_for('filters'))


    def process(self, operation, params):
        func = self.switcher.get(operation, lambda _: 'Invalid Operation')
        return func(params)


    def download(self):
        return sendImage(self.outputImage['img'], self.outputImage['name'])


    def getPoints(self):
        img = self.inputImage['img']
        top_left = '0,0'
        top_right = concat([img.shape[1],',0'])
        bot_left = concat(['0,',img.shape[0]])
        bot_right = concat([img.shape[1],',',img.shape[0]])
        print([top_left, top_right, bot_left, bot_right])
        return [top_left, top_right, bot_left, bot_right]


    def getOpList(self):
        return [i for i in self.switcher.keys()]


    def homography(self, params):
        filename = 'temp_' + self.inputImage['name']

        cv2.imwrite(filename, self.inputImage['img'])
        formatedCoords = concat([\
            concat(list(map(int, params['top_left'].split(sep = ',')))),
            concat(list(map(int, params['bot_left'].split(sep = ',')))),
            concat(list(map(int, params['bot_right'].split(sep = ',')))),
            concat(list(map(int, params['top_right'].split(sep = ','))))
        ])
        command = concat(['homography.exe', filename, formatedCoords, 1])
        os.system(command)
        img = cv2.imread(filename, cv2.IMREAD_COLOR)

        os.remove(filename)
        self.croppedImage['img'] = img
        self.croppedImage['name'] = 'crop_' + self.inputImage['name']
        self.croppedImage['empty'] = False


    def opCornerDetection(self, params):
        filename = 'temp_' + self.inputImage['name']

        print(filename)
        cv2.imwrite(filename, self.inputImage['img'])
        hRadius = 5
        k = 0.04
        sf = 0.1
        threshold = 100.0
        command = concat(['cornerDetection.exe', filename, hRadius, k, sf, threshold])
        os.system(command)
        img = cv2.imread(filename, cv2.IMREAD_COLOR)

        os.remove(filename)
        self.outputImage['img'] = img
        self.outputImage['name'] = 'scan_' + self.inputImage['name']
        self.outputImage['empty'] = False
        return redirect(url_for('scan'))


    def opProcessImage(self, params):
        self.homography(params)

        filename = 'temp_' + self.croppedImage['name']

        cv2.imwrite(filename, self.croppedImage['img'])
        command = concat(['sobelFilter.exe', filename])
        os.system(command)
        img = cv2.imread(filename, cv2.IMREAD_COLOR)

        os.remove(filename)
        self.outputImage['img'] = img
        self.outputImage['name'] = 'scan_' + self.inputImage['name']
        self.outputImage['empty'] = False
        return redirect(url_for('scan'))
