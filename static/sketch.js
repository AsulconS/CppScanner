function scaleImage(width, height, max_width, max_height)
{
    var result = new Array();
    var w_s = width / max_width;
    var h_s = height / max_height;
    if(w_s > h_s && w_s > 1)
    {
        result['width'] = max_width;
        result['height'] = max_width * height / width;
    }
    else if (h_s > w_s && h_s > 1)
    {
        result['width'] = width * max_height / height;
        result['height'] = max_height;
    }
    else
    {
        result['width'] = width;
        result['height'] = height;
    }
    return result;
}

function scaleCoord(rangoInicial, rangoFinal, coord)
{
    var a = rangoInicial[0];
    var c = rangoFinal[0];
    var ri_len = rangoInicial[1] - rangoInicial[0];
    var rf_len = rangoFinal[1] - rangoFinal[0];
    return ((coord - a) * rf_len / ri_len) + c;
}

function scaleImageCoord(coord, initialSize, finalSize)
{
    var new_x = scaleCoord([0, initialSize[0]], [0, finalSize[0]], coord[0]);
    var new_y = scaleCoord([0, initialSize[1]], [0, finalSize[1]], coord[1]);
    return [new_x, new_y];
}

function stringToArray(str_coord)
{
    var coords = str_coord.split(',');
    for (let i = 0; i < coords.length; ++i)
        coords[i] = parseInt(coords[i]);
    return coords;
}

function drawCorner(context, coords)
{
    context.fillStyle = 'rgba(0, 0, 0, 0.25)';
    context.strokeStyle = 'white';
    context.imageSmoothingEnabled = true;
    context.beginPath();
    context.arc(coords[0], coords[1], 5, 0, 2 * Math.PI);
    context.fill();
    context.stroke();
}

function setList(op_array)
{
    op_select = document.getElementById('operation');
    for(let i = 0; i < op_array.length; ++i)
    {
        var opt = document.createElement('option');
        opt.value = i;
        opt.innerHTML = op_array[i].substr(1, op_array[i].length - 2);
        op_select.appendChild(opt);
    }
}

let points;
let form_points;
var canvas,ctx;
var canvasX,canvasY;
var op_list;
var original_size;
var scaled_size;

function setup()
{
    image = document.getElementById('img');
    canvas = document.getElementById('canvas');
    divimg = document.getElementById('imgdiv');
    op_list = document.getElementById('op_list').innerHTML;

    console.log(op_list);
    setList(op_list.substr(1, op_list.length - 2).split(', '));

    ctx = canvas.getContext('2d');
    //noStroke();

    const arr = scaleImage(image.width, image.height, 1000, 600);
    original_size = [image.width, image.height];
    scaled_size = [arr['width'], arr['height']];

    // resizing the image 
    divimg.style.width =  arr['width']+'px';
    divimg.style.height =  arr['height']+'px';
    ctx.canvas.width = arr['width'];
    ctx.canvas.height = arr['height'];
    image.width = arr['width'];
    image.height = arr['height'];

    canvas.addEventListener('mousemove', function(e) { 
        var cRect = canvas.getBoundingClientRect();        // Gets CSS pos, and width/height
        canvasX = Math.round(e.clientX - cRect.left);  // Subtract the 'left' of the canvas 
        canvasY = Math.round(e.clientY - cRect.top);   // from the X/Y positions to make  
    });

    // obteniendo los inputs de las esquinas
    tl = document.getElementById('top_left');
    tr = document.getElementById('top_right');
    bl = document.getElementById('bot_left');
    br = document.getElementById('bot_right');
    form_points = [tl,tr,br,bl];

    // obteniendo los valores de las esquinas
    tl = stringToArray(tl.value);
    tr = stringToArray(tr.value);
    bl = stringToArray(bl.value);
    br = stringToArray(br.value);
    points = [tl,tr,br,bl];
    print(points);
    
    // seteando los valores del form
    for(let i = 0; i < form_points.length; ++i)
        form_points[i].value = parseInt(points[i][0]) + ',' + parseInt(points[i][1]);

    // escalando las coordenadas para el canvas
    for(let i = 0; i < points.length; ++i)
        points[i] = scaleImageCoord(points[i], original_size, scaled_size);
    print('escalados',points);
    
    drawCorner(ctx, tl);
    drawCorner(ctx, tr);
    drawCorner(ctx, bl);
    drawCorner(ctx, br);

}

var selected , move;

function mousePressed()
{
    for(let i = 0; i < points.length; ++i)
    {
        dist = Math.sqrt((points[i][0] - canvasX)**2 + (points[i][1] - canvasY)**2);
        if(dist <= 50)
        {
            selected = i;
            move = true;
            break;
        }
    }
}

function mouseReleased()
{
    move = false;
}

function draw(_)
{
    ctx.clearRect(0, 0, canvas.width, canvas.height);

    ctx.beginPath();
    ctx.moveTo(points[0][0], points[0][1]);
    for(let i = 1; i < points.length; ++i)
        ctx.lineTo(points[i][0],points[i][1]);

    ctx.closePath();
    ctx.fillStyle = 'rgba(0, 0, 0, 0.25)';
    ctx.fill();

    for (let i = 0; i < points.length; ++i)
        drawCorner(ctx, points[i]);
    if(move)
    {
        points[selected] = [canvasX,canvasY];
        value = scaleImageCoord([canvasX,canvasY],scaled_size,original_size);
        form_points[selected].value = parseInt(value[0])+","+parseInt(value[1]);
    }
}
