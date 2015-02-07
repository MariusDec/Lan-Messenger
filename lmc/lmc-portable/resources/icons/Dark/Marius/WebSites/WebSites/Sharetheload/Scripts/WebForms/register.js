var map;
        var myLocation;
        var infowindow;
        var bucharest = new google.maps.LatLng(44.4325, 26.1039);

        function initialize() {
            var mapProp = {
                center: bucharest,
                zoom: 10,
                mapTypeId: google.maps.MapTypeId.ROADMAP
            };

            map = new google.maps.Map(document.getElementById("googleMap"), mapProp);

            myLocation = new google.maps.Marker({
                position: bucharest,
            });

            myLocation.setMap(map);

            document.getElementById('<%=latitudeLabel.ClientID %>').innerHTML = bucharest.lat();
            document.getElementById('<%=longitudeLabel.ClientID %>').innerHTML = bucharest.lng();

            infowindow = new google.maps.InfoWindow({
                content: 'Select your location !'
            });

            infowindow.open(map, myLocation);

            google.maps.event.addListener(map, 'click', function (event) {
                placeMarker(event.latLng);
            });
        }

        function placeMarker(location) {
            myLocation.setPosition(location);
            document.getElementById('<%=latitudeLabel.ClientID %>').innerHTML = '' + location.lat();
            document.getElementById('<%=longitudeLabel.ClientID %>').innerHTML = '' + location.lng();
            PageMethods.UpdateCoordinates(location.lat(), location.lng());
        }

        google.maps.event.addDomListener(window, 'load', initialize);