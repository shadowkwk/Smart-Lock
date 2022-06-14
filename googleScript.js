var SPREADSHEET_URL_KEY = 'your google spreadsheet id';
var SHEET_NAME = 'Sheet1';
var SS = SpreadsheetApp.openById(SPREADSHEET_URL_KEY);
var sheet = SS.getSheetByName(SHEET_NAME);

function doPost(e) {
  var parsedData;
  var result = {};
  try {
    parsedData = JSON.parse(e.postData.contents);
  }
  catch (f) {
    return ContentService.createTextOutput("Error in parsing request body: " + f.message);
  }

  if (parsedData !== undefined) {
    var flag = parsedData.format;
    if (flag === undefined) {
      flag = 0;
    }

    var dataArr = parsedData.values.split(","); // creates an array of the values to publish 

    var T1 = dataArr[0];
    var T2 = dataArr[1];

    if (T1 !== undefined && T2 !== undefined) {
      var row = sheet.getLastRow() + 1;
      var t = new Date();
      if (T1 == 0)
        sheet.getRange("B" + row).setValue("Incorrect");
      if (T1 == 1)
        sheet.getRange("B" + row).setValue("Correct");
      if (T2 == 0)
        sheet.getRange("C" + row).setValue("Not accessed");
      if (T2 == 1)
        sheet.getRange("C" + row).setValue("RFID card");
      if (T2 == 2)
        sheet.getRange("C" + row).setValue("Keypad");
      sheet.getRange("A" + row).setValue(t);
    }

    /*sheet.getRange("A15").setValue(sheet.getRange("A2").getValue().toString());
    if (sheet.getRange("A2").getValue().toString() == "Incorrest")
      sheet.getRange("A15").setValue("good");*/

    var warning = 0;

    if (row >= 6) {
      if (sheet.getRange("B" + (row - 4)).getValue().toString() == "Incorrect")
        warning++;
      if (sheet.getRange("B" + (row - 3)).getValue().toString() == "Incorrect")
        warning++;
      if (sheet.getRange("B" + (row - 2)).getValue().toString() == "Incorrect")
        warning++;
      if (sheet.getRange("B" + (row - 1)).getValue().toString() == "Incorrect")
        warning++;
      if (sheet.getRange("B" + (row)).getValue().toString() == "Incorrect")
        warning++;

      if (warning >= 5) {
        var emailAddress = 'wkkam9@gapps.cityu.edu.hk';
        var subject = 'Smart door system: WARNING!';
        var message = 'We have sent you this email since we detected five unsuccessful attempts in a row in your smart door system. You may change the door permission on the mobile app in order to avoid it.';
        MailApp.sendEmail(emailAddress, subject, message);
      }
    }

    return ContentService.createTextOutput("Done");

  }
}