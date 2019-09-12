'use strict'


// C library API
const ffi = require('ffi');

// Express App (Routes)
const express = require("express");
const app     = express();
const path    = require("path");
const fileUpload = require('express-fileupload');

app.use(fileUpload());

const mysql = require('mysql');
var connection = null;
// Minimization
const fs = require('fs');
const JavaScriptObfuscator = require('javascript-obfuscator');

// Important, pass in port as in `npm run dev 1234`, do not change
const portNum = process.argv[2];
var filesize;
var eventsize;
var alarmsize;
// Send HTML at root, do not change
app.get('/',function(req,res){
  res.sendFile(path.join(__dirname+'/public/index.html'));
});

// Send Style, do not change
app.get('/style.css',function(req,res){
  //Feel free to change the contents of style.css to prettify your Web app
  res.sendFile(path.join(__dirname+'/public/style.css'));
});

// Send obfuscated JS, do not change
app.get('/index.js',function(req,res){
  fs.readFile(path.join(__dirname+'/public/index.js'), 'utf8', function(err, contents) {
    const minimizedContents = JavaScriptObfuscator.obfuscate(contents, {compact: true, controlFlowFlattening: true});
    res.contentType('application/javascript');
    res.send(minimizedContents._obfuscatedCode);
  });
});

//Respond to POST requests that upload files to uploads/ directory
app.post('/upload', function(req, res) {
  if(!req.files) {
    return res.status(400).send('No files were uploaded.');
  }

  let uploadFile = req.files.uploadFile;
  let test  = uploadFile.name.substr( (uploadFile.name.lastIndexOf('.') +1) )
  // Use the mv() method to place the file somewhere on your server
  if(test === "ics")
  {
    uploadFile.mv('uploads/' + uploadFile.name, function(err) {
      if(err)
      {
        return res.status(500).send(err);
      }


      res.redirect('/');
    });
  }
  else
  {
    res.redirect('/');
  }

});

//Respond to GET requests for files in the uploads/ directory
app.get('/uploads/:name', function(req , res){
  fs.stat('uploads/' + req.params.name, function(err, stat) {
    console.log(err);
    if(err == null) {
      res.sendFile(path.join(__dirname+'/uploads/' + req.params.name));
    } else {
      res.send('');
    }
  });
});


//Sample endpoint
app.get('/someendpoint', function(req , res){
  res.send({
    foo: "bar"
  });
});

app.get('/getFileData', function(req , res){
  let libParser = ffi.Library('./libParser', {
    'getCalInfo': [ 'string', [ 'string' ] ],		//return type first, argument list second
    'isValidCalendar': [ 'int', [ 'string' ] ]
  });
  var results = [];
  var dir = './uploads';
  fs.readdir(dir, function(err, files) {
    files.forEach(function (file) {
      if(libParser.isValidCalendar("./uploads/" + file) === 1)
      {
        let itemStr = libParser.getCalInfo("./uploads/" + file);
        let itemObj = JSON.parse(itemStr);
        itemObj.fileName = file;
        results.push(itemObj);
      }
      else
      {
        fs.unlink("./uploads/" + file, function(error) {
          console.log("deleted " + file + " (invalid calendar)");
        });
      }
    });
    res.send(results);
  });

});

app.get('/getEventData', function(req , res){
  let libParser = ffi.Library('./libParser', {
    'getCalInfo': [ 'string', [ 'string' ] ],		//return type first, argument list second
    'isValidCalendar': [ 'int', [ 'string' ] ],
    'getEventInfo': [ 'string', [ 'string' ] ]
  });
  if(libParser.isValidCalendar("./uploads/" + req.query.fileN))
  {
    var results = libParser.getEventInfo("./uploads/" + req.query.fileN)
    res.send(results);
  }
  else
  {
    res.send(JSON.parse("[]"));
  }
});

app.get('/getAlarmData', function(req , res){
  let libParser = ffi.Library('./libParser', {
    'getCalInfo': [ 'string', [ 'string' ] ],		//return type first, argument list second
    'isValidCalendar': [ 'int', [ 'string' ] ],
    'getAlarmList': [ 'string', [ 'string', 'int' ] ]
  });
  if(libParser.isValidCalendar("./uploads/" + req.query.fileN))
  {
    var results = libParser.getAlarmList("./uploads/" + req.query.fileN, req.query.alarmNum);
    res.send(results);
  }
  else
  {
    res.send(JSON.parse("[]"));
  }
});

app.get('/getPropData', function(req , res){
  let libParser = ffi.Library('./libParser', {
    'getCalInfo': [ 'string', [ 'string' ] ],		//return type first, argument list second
    'isValidCalendar': [ 'int', [ 'string' ] ],
    'getPropList': [ 'string', [ 'string', 'int' ] ]
  });
  if(libParser.isValidCalendar("./uploads/" + req.query.fileN))
  {
    var results = libParser.getPropList("./uploads/" + req.query.fileN, req.query.eventNum);
    res.send(results);
  }
  else
  {
    res.send(JSON.parse("[]"));
  }
});

app.get('/addEventToCal', function(req , res){
  let libParser = ffi.Library('./libParser', {
    'isValidCalendar': [ 'int', [ 'string' ] ],
    'addEventToCal': [ 'int', [ 'string', 'string' ] ]
  });
  if(libParser.isValidCalendar("./uploads/" + req.query.fileN))
  {
    if(libParser.addEventToCal("./uploads/" + req.query.fileN,JSON.stringify(req.query.data)) == 1)
    {
      res.send(true);
    }
    else
    {
      res.send(false);
    }
  }
  else
  {
    res.send(false);
  }
});

app.get('/createCal', function(req , res){
  let libParser = ffi.Library('./libParser', {
    'isValidCalendar': [ 'int', [ 'string' ] ],
    'createCalEvent': [ 'int', [ 'string', 'string', 'string' ] ]
  });

  if(libParser.createCalEvent("./uploads/" + req.query.fileN ,JSON.stringify(req.query.Edata), JSON.stringify(req.query.Cdata)) == 1)
  {
    res.send(true);
  }
  else
  {
    res.send(false);
  }const mysql = require('mysql');

});

app.get('/deleteFile', function(req , res){
  fs.unlink("./uploads/" + req.query.fileN, function(error) {
    console.log("deleted " + req.query.fileN + " (user request)");
  });
  res.redirect('/');
});

app.get('/login', function(req , res){
  connection = mysql.createConnection({
    host:'dursley.socs.uoguelph.ca',
    user: req.query.user,
    password : req.query.password,
    database : req.query.dbname
  });
  connection.connect(function(err) {
    if(err)
    {
      console.log("Invalid database login");
      res.send(false);
    }
    else {
      connection.query("create table FILE (cal_id int not null auto_increment,  file_Name varchar(60) not null,  version int not null, prod_id varchar(256) not null, primary key(cal_id) )", function (err, rows, fields) {
      });
      connection.query("create table EVENT (event_id int not null auto_increment,  summary varchar(1024), start_time datetime not null,  location varchar(60), organizer varchar(256), cal_file int not null, primary key(event_id),foreign key(cal_file) references FILE(cal_id) on delete cascade)", function (err, rows, fields) {
      });
      connection.query("create table ALARM (alarm_id int not null auto_increment,  action varchar(256) not null, `trigger` varchar(256) not null, event int not null, primary key(alarm_id), foreign key(event) references EVENT(event_id) on delete cascade)", function (err, rows, fields) {
      });

      res.send(true);
    }
  }

);
});

app.get('/deleteFile', function(req , res){
  fs.unlink("./uploads/" + req.query.fileN, function(error) {
    console.log("deleted " + req.query.fileN + " (user request)");
  });
  res.redirect('/');
});

app.get('/statusDisplay', function(req , res){
  connection.query("SELECT * FROM FILE", function (err, rows, fields) {
    if (err)
    {
      console.log("Something went wrong. "+ err);
    }
    else
    {
      filesize = rows.length;
      connection.query("SELECT * FROM EVENT", function (err, rows, fields) {
        if (err)
        {
          console.log("Something went wrong. "+ err);
        }
        else
        {
          eventsize = rows.length;
          connection.query("SELECT * FROM ALARM", function (err, rows, fields) {
            if (err)
            {
              console.log("Something went wrong. "+ err);
            }
            else
            {
              alarmsize = rows.length;
              res.send("There are " + filesize + " Files, " + eventsize + " Events, and " + alarmsize + " Alarms in the Database");
            }
          });

        }
      });

    }
  });
});

app.get('/uploadToDB', function(req , res){
  connection.query("DELETE FROM FILE", function (err) {
  });
  connection.query("DELETE FROM EVENT", function (err) {
  });
  connection.query("DELETE FROM ALARM", function (err) {
  });

  let libParser = ffi.Library('./libParser', {
    'getCalInfo': [ 'string', [ 'string' ] ],		//return type first, argument list second
    'isValidCalendar': [ 'int', [ 'string' ] ],
    'getEventInfo': [ 'string', [ 'string' ] ],
    'getAlarmList': [ 'string', [ 'string', 'int' ] ]
  });

  var dir = './uploads';
  fs.readdir(dir, function(err, files) {
    files.forEach(function (file) {
      if(libParser.isValidCalendar("./uploads/" + file) === 1)
      {
        let itemStr = libParser.getCalInfo("./uploads/" + file);
        let itemObj = JSON.parse(itemStr);
        itemObj.fileName = file;
        connection.query("INSERT INTO FILE (file_Name, version, prod_id) VALUES ('" + itemObj.fileName + "', '" + itemObj.version + "', '" + itemObj.prodID +"')", function (err) {
          if(err)
          {
          }
          else {
            var eventSTR = libParser.getEventInfo("./uploads/" + itemObj.fileName);
            var eventOBJ = JSON.parse(eventSTR);
            let count = 0;
            eventOBJ.forEach(function(arrayItem) {
              connection.query("INSERT INTO EVENT (summary, start_time, location, organizer, cal_file) VALUES ('" + arrayItem.summary + "', '" + arrayItem.version + "', '" + arrayItem.location + "', '" + arrayItem.organizer + "', (SELECT cal_id FROM FILE WHERE file_Name = '" + itemObj.fileName +"'))", function (err) {
                if(err)
                {
                  console.log(err);
                }
                else
                {
                  var alarmsSTR = libParser.getAlarmList("./uploads/" + itemObj.fileName, 0);
                  let alarmOBJ = JSON.parse(alarmsSTR);
                  alarmOBJ.forEach(function(arrayI) {
                    connection.query("INSERT INTO ALARM (action, `trigger`, event) VALUES ('" + arrayI.action + "', '" + arrayI.trigger + "', (SELECT MAX(event_id) FROM EVENT))", function (err) {
                      if(err)
                      {
                        console.log(err);
                      }
                    });
                  });
                }
              });
              count++;
            });
          }
        });
      }
      else
      {
        fs.unlink("./uploads/" + file, function(error) {
          console.log("deleted " + file + " (invalid calendar)");
        });
      }
    });
    res.send("results");
  });
});

app.get('/clearDB', function(req , res){
  connection.query("DELETE FROM FILE", function (err) {
  });
  connection.query("DELETE FROM EVENT", function (err) {
  });
  connection.query("DELETE FROM ALARM", function (err) {
  });
  res.send(true)
});

  app.listen(portNum);
  console.log('Running app at localhost: ' + portNum);
