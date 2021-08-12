function scaleImage(width, height, max_width, max_height) {
    console.log("asd", width, height)
    var result = new Array();
    var w_s = width / max_width;
    var h_s = height / max_height;
    if (w_s > h_s && w_s > 1) {
        result['width'] = max_width;
        result['height'] = max_width * height / width;
    }
    else if (h_s > w_s && h_s > 1) {
        result['width'] = width * max_height / height;
        result['height'] = max_height;
    }
    else {
        result['width'] = width;
        result['height'] = height;
    }
    console.log("asd", result['width'], result['height'])
    return result;
}

function setup() {
    console.log("asdasd")
    var image = document.getElementById('img');
    var divimg = document.getElementById('imgdiv');

    const arr = scaleImage(image.width, image.height, 1000, 600);
    divimg.style.width =  arr['width']+"px";
    divimg.style.height =  arr['height']+"px";
    image.width = arr['width']
    image.height = arr['height']

}

