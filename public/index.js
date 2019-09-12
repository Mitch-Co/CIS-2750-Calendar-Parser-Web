// Put all onload AJAX calls here, and event listeners
$(document).ready(function() {
  // On page-load AJAX Example
  $.ajax({
    type: 'get',            //Request type
    dataType: 'json',       //Data type - we will use JSON for almost everything
    url: '/getFileData',   //The server endpoint we are connecting to
    success: function (data) {
      /*  Do something with returned object
      Note that what we get is an object, not a string,
      so we do not need to parse it on the server.
      JavaScript really does handle JSONs seamlessly
      */
      data.forEach(function (arrayItem) {
        var row = document.createElement("TR");
        row.className += "toDelete0";
        var e1  = document.createElement("TH");
        var link = document.createElement("A");
        link.href = "/uploads/" + arrayItem.fileName;
        link.text = arrayItem.fileName;
        e1.appendChild(link);
        var e2  = document.createElement("TH");
        e2.appendChild(document.createTextNode(arrayItem.version));
        var e3  = document.createElement("TH");
        e3.appendChild(document.createTextNode(arrayItem.prodID));
        var e4  = document.createElement("TH");
        e4.appendChild(document.createTextNode(arrayItem.numEvents));
        var e5  = document.createElement("TH");
        e5.appendChild(document.createTextNode(arrayItem.numProps));
        row.appendChild(e1);
        row.appendChild(e2);
        row.appendChild(e3);
        row.appendChild(e4);
        row.appendChild(e5);
        document.getElementById("table1").appendChild(row);
        $('#select').append("<option class=\"toDelete0\" value=\"" + arrayItem.fileName+"\">" + arrayItem.fileName +"</option>");

      });
      if(JSON.stringify(data) === "{}" || JSON.stringify(data) === "[]" )
      {
        var row = document.createElement("TR");
        row.className += "toDelete0";
        var e1  = document.createElement("TH");
        e1.appendChild(document.createTextNode("EMPTY"));
        var e2  = document.createElement("TH");
        e2.appendChild(document.createTextNode("EMPTY"));
        var e3  = document.createElement("TH");
        e3.appendChild(document.createTextNode("EMPTY"));
        var e4  = document.createElement("TH");
        e4.appendChild(document.createTextNode("EMPTY"));
        var e5  = document.createElement("TH");
        e5.appendChild(document.createTextNode("EMPTY"));
        row.appendChild(e1);
        row.appendChild(e2);
        row.appendChild(e3);
        row.appendChild(e4);
        row.appendChild(e5);
        document.getElementById("table1").appendChild(row);
        $('#select').append("<option class=\"toDelete0\" value=\"NONE\">NONE</option>");
        $('#statusButton3').prop('disabled', true);
      }
      else
      {
        let sel = document.getElementById('select');
        updateCEP(sel.options[sel.selectedIndex].text);
      }
      statusUpdate("File Log Panel updated to reflect server data");
      disableServerPanel();
      //We write the object to the console to show that the request was successful
      console.log(data);

    },
    fail: function(error) {
      // Non-200 return, do something with error
      console.log(error);
      statusUpdate("ERROR LOADING FILE LOG PANEL");
    }
  });

  function refreshFLP()
  {
    $(".toDelete0").remove();
    $.ajax({
      type: 'get',            //Request type
      dataType: 'json',       //Data type - we will use JSON for almost everything
      url: '/getFileData',   //The server endpoint we are connecting to
      success: function (data) {

        data.forEach(function (arrayItem) {
          var row = document.createElement("TR");
          row.className += "toDelete0";
          var e1  = document.createElement("TH");
          var link = document.createElement("A");
          link.href = "/uploads/" + arrayItem.fileName;
          link.text = arrayItem.fileName;
          e1.appendChild(link);
          var e2  = document.createElement("TH");
          e2.appendChild(document.createTextNode(arrayItem.version));
          var e3  = document.createElement("TH");
          e3.appendChild(document.createTextNode(arrayItem.prodID));
          var e4  = document.createElement("TH");
          e4.appendChild(document.createTextNode(arrayItem.numEvents));
          var e5  = document.createElement("TH");
          e5.appendChild(document.createTextNode(arrayItem.numProps));
          row.appendChild(e1);
          row.appendChild(e2);
          row.appendChild(e3);
          row.appendChild(e4);
          row.appendChild(e5);
          document.getElementById("table1").appendChild(row);
          $('#select').append("<option class=\"toDelete0\" value=\"" + arrayItem.fileName+"\">" + arrayItem.fileName +"</option>");

        });
        if(JSON.stringify(data) === "{}" || JSON.stringify(data) === "[]" )
        {
          var row = document.createElement("TR");
          row.className += "toDelete0";
          var e1  = document.createElement("TH");
          e1.appendChild(document.createTextNode("EMPTY"));
          var e2  = document.createElement("TH");
          e2.appendChild(document.createTextNode("EMPTY"));
          var e3  = document.createElement("TH");
          e3.appendChild(document.createTextNode("EMPTY"));
          var e4  = document.createElement("TH");
          e4.appendChild(document.createTextNode("EMPTY"));
          var e5  = document.createElement("TH");
          e5.appendChild(document.createTextNode("EMPTY"));
          row.appendChild(e1);
          row.appendChild(e2);
          row.appendChild(e3);
          row.appendChild(e4);
          row.appendChild(e5);
          document.getElementById("table1").appendChild(row);
          $('#select').append("<option class=\"toDelete0\" value=\"NONE\">NONE</option>");
          $('#statusButton3').prop('disabled', true);
        }
        else
        {
          let sel = document.getElementById('select');
          updateCEP(sel.options[sel.selectedIndex].text);
        }
        statusUpdate("File Log Panel updated to reflect server data");

        //We write the object to the console to show that the request was successful
        console.log(data);

      },
      fail: function(error) {
        // Non-200 return, do something with error
        console.log(error);
        statusUpdate("ERROR LOADING FILE LOG PANEL");
      }
    });
  }

  $('input:file').on("change", function() {
    $('#submt').prop('disabled', !$(this).val());
  });

  $('#select').on("change", function() {
    let sel = document.getElementById('select');
    updateCEP(sel.options[sel.selectedIndex].text);
  });

  $('#clearDB').click( function() {
    $.ajax({
      type: 'get',
      url: '/clearDB',   //The server endpoint we are connecting to
      success: function () {
        statusUpdate("Database cleared");
        displayServerStatus();
      },
      fail: function(error) {
        console.log(error);
        statusUpdate("ERROR FINDING DATABASE DATA");
      }
    });
  });


  $('#displayStatus').click( function() {
    displayServerStatus();
  });

  function displayServerStatus() {
    $.ajax({
      type: 'get',
      url: '/statusDisplay',   //The server endpoint we are connecting to
      success: function (data) {
        statusUpdate(data);
      },
      fail: function(error) {
        console.log(error);
        statusUpdate("ERROR FINDING DATABASE DATA");
      }
    });
  }

  $('#select2').on("change", function() {
    let sel = document.getElementById('select');
    let seltwo = document.getElementById('select2');
    updateAlarmP(sel.options[sel.selectedIndex].text,seltwo.options[seltwo.selectedIndex].text);
  });

  $('#select3').on("change", function() {
    let sel = document.getElementById('select');
    let selthree = document.getElementById('select3');
    updateEventP(sel.options[sel.selectedIndex].text,selthree.options[selthree.selectedIndex].text);
  });

  $('#submt').click( function() {
    statusUpdate("Attempting to upload file ");
    refreshFLP();
  });

  $('#form3').submit(function(e){
    e.preventDefault();
    statusUpdate("Attempting to log in to Database");
    $.ajax({
      type: 'get',
      data: JSON.parse(
        "{\"user\":\"" + $('#USR1').val() + "\"," +
        "\"password\":\"" + $('#PSW1').val()  + "\"," +
        "\"dbname\":\"" + $('#DID1').val()  + "\"}"
    ),
      url: '/login',   //The server endpoint we are connecting to
      success: function (data) {
        if(data == true)
        {
          statusUpdate("Login Successful");
          $('#login').prop('disabled', true);
          enableServerPanel();

        }
        else
        {
          statusUpdate("Login Unsuccessful");
        }
      },
      fail: function(error) {
        console.log(error);
        statusUpdate("ERROR LOGGING IN");
      }
    });
  });


  $('#uploadToDB').click( function() {
    var sel = document.getElementById('select');
    $.ajax({
      type: 'get',
      url: '/uploadToDB',   //The server endpoint we are connecting to
      success: function () {
        statusUpdate("Database updated with current files");
        displayServerStatus();
      },
      fail: function(error) {
        console.log(error);
        statusUpdate("ERROR UPLOADING");
      }
    });
  });

  $('#statusButton3').click( function() {
    var sel = document.getElementById('select');
    $.ajax({
      type: 'get',
      data: JSON.parse("{\"fileN\":\"" + sel.options[sel.selectedIndex].text + "\"}"),
      url: '/deleteFile',   //The server endpoint we are connecting to
      success: function () {
        /*  Do something with returned object
        Note that what we get is an object, not a string,
        so we do not need to parse it on the server.
        JavaScript really does handle JSONs seamlessly
        */
        location.reload();
      },
      fail: function(error) {
        console.log(error);
        statusUpdate("ERROR");
      }
    });
  });


  function appendZero(textInput){
    if(textInput < 10)
    {
      textInput = "0" + textInput;
    }

    return textInput;
  }

  function statusUpdate(textInput){
    var time = new Date();
    $('#statusPanel').append("<p>" + appendZero(time.getHours()) + ":" + appendZero(time.getMinutes())  + " - " + textInput + "<\p>");
    updateScroll();
  }

  function updateScroll(){
    var element = document.getElementById("statusPanel");
    element.scrollTop = element.scrollHeight;
  }

  function updateEventP(fileN, eventNum)
  {
    $.ajax({
      type: 'get',            //Request type
      dataType: 'json',       //Data type - we will use JSON for almost everything
      data: JSON.parse("{\"fileN\":\"" + fileN + "\",\"eventNum\":\"" + eventNum + "\"}"),
      url: '/getPropData',   //The server endpoint we are connecting to
      success: function (data) {
        /*  Do something with returned object
        Note that what we get is an object, not a string,
        so we do not need to parse it on the server.
        JavaScript really does handle JSONs seamlessly
        */
        statusUpdate("Loading " + fileN + "; event " + eventNum +"'s properties");
        var count = 0;
        $(".toDelete4").remove();
        data.forEach(function (arrayItem) {
          let row = document.createElement("TR");
          row.className += "toDelete4";
          let e1  = document.createElement("TH");
          e1.appendChild(document.createTextNode(count));
          let e2  = document.createElement("TH");
          e2.appendChild(document.createTextNode(arrayItem.propName));
          let e3  = document.createElement("TH");
          e3.appendChild(document.createTextNode(arrayItem.propDescr));
          row.appendChild(e1);
          row.appendChild(e2);
          row.appendChild(e3);
          document.getElementById("table4").appendChild(row);
          count += 1;

        });
        if(JSON.stringify(data) === "{}" || JSON.stringify(data) === "[]")
        {
          let row = document.createElement("TR");
          row.className += "toDelete4";
          let e1 = document.createElement("TH");
          e1.appendChild(document.createTextNode("EMPTY"));
          let e2 = document.createElement("TH");
          e2.appendChild(document.createTextNode("EMPTY"));
          let e3 = document.createElement("TH");
          e3.appendChild(document.createTextNode("EMPTY"));
          row.appendChild(e1);
          row.appendChild(e2);
          row.appendChild(e3);
          document.getElementById("table4").appendChild(row);
        }
        //We write the object to the console to show that the request was successful
        console.log(data);
      },
      fail: function(error) {
        console.log(error);
        statusUpdate("ERROR LOADING PROP PANEL");
      }
    });
  }

  function updateAlarmP(fileN , alarmNum){
    $.ajax({
      type: 'get',            //Request type
      dataType: 'json',       //Data type - we will use JSON for almost everything
      data: JSON.parse("{\"fileN\":\"" + fileN + "\",\"alarmNum\":\"" + alarmNum + "\"}"),
      url: '/getAlarmData',   //The server endpoint we are connecting to
      success: function (data) {
        /*  Do something with returned object
        Note that what we get is an object, not a string,
        so we do not need to parse it on the server.
        JavaScript really does handle JSONs seamlessly
        */
        statusUpdate("Loading " + fileN + "; event " + alarmNum +"'s alarms");
        var count = 0;
        $(".toDelete3").remove();
        data.forEach(function (arrayItem) {
          let row = document.createElement("TR");
          row.className += "toDelete3";
          let e1  = document.createElement("TH");
          e1.appendChild(document.createTextNode(count));
          let e2  = document.createElement("TH");
          e2.appendChild(document.createTextNode(arrayItem.action));
          let e3  = document.createElement("TH");
          e3.appendChild(document.createTextNode(arrayItem.trigger));
          let e4  = document.createElement("TH");
          e4.appendChild(document.createTextNode(arrayItem.numProps));
          row.appendChild(e1);
          row.appendChild(e2);
          row.appendChild(e3);
          row.appendChild(e4);
          document.getElementById("table3").appendChild(row);
          count += 1;

        });
        if(JSON.stringify(data) === "{}" || JSON.stringify(data) === "[]")
        {
          let row = document.createElement("TR");
          row.className += "toDelete3";
          let e1 = document.createElement("TH");
          e1.appendChild(document.createTextNode("EMPTY"));
          let e2 = document.createElement("TH");
          e2.appendChild(document.createTextNode("EMPTY"));
          let e3 = document.createElement("TH");
          e3.appendChild(document.createTextNode("EMPTY"));
          let e4 = document.createElement("TH");
          e4.appendChild(document.createTextNode("EMPTY"));
          row.appendChild(e1);
          row.appendChild(e2);
          row.appendChild(e3);
          row.appendChild(e4);
          document.getElementById("table3").appendChild(row);
        }
        //We write the object to the console to show that the request was successful
        console.log(data);
      },
      fail: function(error) {
        console.log(error);
        statusUpdate("ERROR LOADING ALARM PANEL");
      }
    });
  }

  function updateCEP(fileN)
  {
    $.ajax({
      type: 'get',            //Request type
      dataType: 'json',       //Data type - we will use JSON for almost everything
      data: JSON.parse("{\"fileN\":\"" + fileN + "\"}"),
      url: '/getEventData',   //The server endpoint we are connecting to
      success: function (data) {
        /*  Do something with returned object
        Note that what we get is an object, not a string,
        so we do not need to parse it on the server.
        JavaScript really does handle JSONs seamlessly
        */
        statusUpdate("Calendar Event Panel updated for events in " + fileN);
        var count = 0;
        $(".toDelete").remove();
        $(".toDelete2").remove();
        $(".toDelete3").remove();
        data.forEach(function (arrayItem) {
          let row = document.createElement("TR");
          row.className += "toDelete";
          let e1  = document.createElement("TH");
          e1.appendChild(document.createTextNode(count));
          let e2  = document.createElement("TH");
          e2.appendChild(document.createTextNode(arrayItem.startDT.date.slice(0,4) + "/" + arrayItem.startDT.date.slice(4,6) + "/" +arrayItem.startDT.date.slice(6,8)));
          let e3  = document.createElement("TH");
          if(arrayItem.startDT.isUTC)
          {
            e3.appendChild(document.createTextNode(arrayItem.startDT.time.slice(0,2) + ":" + arrayItem.startDT.time.slice(2,4) +":" + arrayItem.startDT.time.slice(4,6) + " (UTC)"));
          }
          else
          {
            e3.appendChild(document.createTextNode(arrayItem.startDT.time.slice(0,2) + ":" + arrayItem.startDT.time.slice(2,4) +":" + arrayItem.startDT.time.slice(4,6)));
          }

          let e4  = document.createElement("TH");
          e4.appendChild(document.createTextNode(arrayItem.summary));
          let e5  = document.createElement("TH");
          e5.appendChild(document.createTextNode(arrayItem.numProps));
          let e6  = document.createElement("TH");
          e6.appendChild(document.createTextNode(arrayItem.numAlarms));
          row.appendChild(e1);
          row.appendChild(e2);
          row.appendChild(e3);
          row.appendChild(e4);
          row.appendChild(e5);
          row.appendChild(e6);
          document.getElementById("table2").appendChild(row);
          $('#select2').append("<option class=\"toDelete2\" value=\"" + arrayItem.fileName+ count + "\">" + count +"</option>");
          $('#select3').append("<option class=\"toDelete2\" value=\"" + arrayItem.fileName+ count + "\">" + count +"</option>");
          count += 1;

        });
        if(JSON.stringify(data) === "{}" || JSON.stringify(data) === "[]")
        {
          let row = document.createElement("TR");
          row.className += "toDelete";
          let e1 = document.createElement("TH");
          e1.appendChild(document.createTextNode("EMPTY"));
          let e2 = document.createElement("TH");
          e2.appendChild(document.createTextNode("EMPTY"));
          let e3 = document.createElement("TH");
          e3.appendChild(document.createTextNode("EMPTY"));
          let e4 = document.createElement("TH");
          e4.appendChild(document.createTextNode("EMPTY"));
          let e5 = document.createElement("TH");
          e5.appendChild(document.createTextNode("EMPTY"));
          let e6 = document.createElement("TH");
          e6.appendChild(document.createTextNode("EMPTY"));
          row.appendChild(e1);
          row.appendChild(e2);
          row.appendChild(e3);
          row.appendChild(e4);
          row.appendChild(e5);
          row.appendChild(e6);
          document.getElementById("table2").appendChild(row);
        }
        else
        {
          let sel = document.getElementById('select');
          let seltwo = document.getElementById('select2');
          updateAlarmP(sel.options[sel.selectedIndex].text,seltwo.options[seltwo.selectedIndex].text);
          let selthree = document.getElementById('select3');
          updateEventP(sel.options[sel.selectedIndex].text,selthree.options[selthree.selectedIndex].text);
        }
        //We write the object to the console to show that the request was successful
        console.log(data);

      },
      fail: function(error) {
        console.log(error);
        statusUpdate("ERROR LOADING EVENT PANEL");
      }
    });



  }

  function getCurrentDT()
  {
    var time = new Date();
    let ret = time.getFullYear() + appendZero(time.getMonth() + 1) + appendZero(time.getDay()) +  "T" + appendZero(time.getHours()) + appendZero(time.getMinutes()) + appendZero(time.getSeconds());
    return ret;
  }
  $('#form1').submit(function(e){
    e.preventDefault();

    let sel = document.getElementById('select');
    let yeet = sel.options[sel.selectedIndex].text;
    let date = $('#DAT1').val();
    let time = $('#TIM1').val();
    var dt1 = "";

    if(!isNaN(date) && !isNaN(time) && date.length == 8  && time.length == 4)
    {
      dt1 = date + "T" + time + "00";
      if($('#UTC1').prop("checked") == true)
      {
        dt1 += "Z";
      }
    }
    else
    {
      statusUpdate("Incorrect DT Format Entered in Event Form");
      return;
    }
    let dat = JSON.parse("{\"fileN\":\"" + yeet + "\",\"data\":{" +
    "\"UID\":\"" + $('#UID1').val() + "\"," +
    "\"DAT1\":\"" + dt1 + "\"," +
    "\"DAT2\":\"" + getCurrentDT() + "\"," +
    "\"SUM\":\"" + $('#SUM1').val() + "\"}}"
  );
  statusUpdate("Attempting to add Event to Calendar " + JSON.stringify(dat));

  $.ajax({
    type: 'get',            //Request type
    dataType: 'json',       //Data type - we will use JSON for almost everything
    data: dat,
    url: '/addEventToCal',   //The server endpoint we are connecting to
    success: function (data) {
      if(data == true)
      {
        let sel = document.getElementById('select');
        refreshFLP();
      }
      else
      {
        statusUpdate("Event failed to be added to " + dat.fileN);
      }
      console.log(data);

    },
    fail: function(error) {
      console.log(error);
      statusUpdate("ERROR ADDING EVENT");
    }
  });
});

$('#form2').submit(function(e){
  e.preventDefault();

  let yeet = $('#FIL').val()
  let date = $('#DAT2').val();
  let time = $('#TIM2').val();
  var dt1 = "";

  let test = yeet.substr( (yeet.lastIndexOf('.') +1) );
  if(test !== "ics")
  {
    statusUpdate("Filename does not contain .ics");
    return;
  }

  if(!isNaN(date) && !isNaN(time) && date.length == 8  && time.length == 4)
  {
    dt1 = date + "T" + time + "00";
    if($('#UTC1').prop("checked") == true)
    {
      dt1 += "Z";
    }
  }
  else
  {
    statusUpdate("Incorrect DT Format Entered in Event Form");
    return;
  }
  let dat = JSON.parse("{\"fileN\":\"" + yeet + "\",\"Edata\":{" +
  "\"UID\":\"" + $('#UID2').val() + "\"," +
  "\"DAT1\":\"" + dt1 + "\"," +
  "\"DAT2\":\"" + getCurrentDT() + "\"," +
  "\"SUM\":\"" + $('#SUM2').val() + "\"},\"Cdata\":{" +
  "\"version\":\"" + $('#VER').val()  + "\"," +
  "\"prodID\":\""  + $('#PID').val() + "\"}}"
);
statusUpdate("Attempting to Create Calendar " + JSON.stringify(dat));

$.ajax({
  type: 'get',            //Request type
  dataType: 'json',       //Data type - we will use JSON for almost everything
  data: dat,
  url: '/createCal',   //The server endpoint we are connecting to
  success: function (data) {
    if(data == true)
    {
      let sel = document.getElementById('select');
      refreshFLP();
      statusUpdate("Calendar" + dat.fileN + " Successfuly Created");
    }
    else
    {
      statusUpdate("Calendar" + dat.fileN + " Failed to be Created " + JSON.stringify(dat.Cdata));
    }
    console.log(data);
  },
  fail: function(error) {
    console.log(error);
    statusUpdate("ERROR CREATING CALENDAR");
  }
});
});

function disableServerPanel() {
$('#uploadToDB').prop('disabled', true);
$('#clearDB').prop('disabled', true);
$('#displayStatus').prop('disabled', true);
$('#Q1').prop('disabled', true);
$('#Q2').prop('disabled', true);
$('#Q3').prop('disabled', true);
$('#Q4').prop('disabled', true);
$('#Q5').prop('disabled', true);
$('#Q6').prop('disabled', true);
}

function enableServerPanel() {
  $('#uploadToDB').prop('disabled', false);
  $('#clearDB').prop('disabled', false);
  $('#displayStatus').prop('disabled', false);
  $('#Q1').prop('disabled', false);
  $('#Q2').prop('disabled', false);
  $('#Q3').prop('disabled', false);
  $('#Q4').prop('disabled', false);
  $('#Q5').prop('disabled', false);
  $('#Q6').prop('disabled', false);
}



});
