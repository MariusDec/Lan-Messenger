$.ajax({
                type: "POST",
                url: "MyWebService.asmx/SayHello",
                data: "{firstName:'Aidy', lastName:'F'}", // the data in JSON format.  Note it is *not* a JSON object, is is a literal string in JSON format
                contentType: "application/json; charset=utf-8", // we are sending in JSON format so we need to specify this
                dataType: "json", // the data type we want back.  The data will come back in JSON format
                success: function (data) {
                    $("#searchresultsB").html(data.d); // it's a quirk, but the JSON data comes back in a property called "d"; {"d":"Hello Aidy F"}
                }
            });