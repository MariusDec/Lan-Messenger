<%@ Control Language="C#" AutoEventWireup="true" CodeFile="MapControl.ascx.cs" Inherits="Controls_MapControl" %>

<script src="http://maps.googleapis.com/maps/api/js"></script>
    <script>
        var map
        var myCity
        var bucharest = new google.maps.LatLng(44.4325, 26.1039);

        function initialize() {
            var mapProp = {
                center: bucharest,
                zoom: 10,
                mapTypeId: google.maps.MapTypeId.ROADMAP
            };

            map = new google.maps.Map(document.getElementById("googleMap"), mapProp);

            myCity = new google.maps.Circle({
                center: bucharest,
                radius: 5000,
                strokeColor: "#0000FF",
                strokeOpacity: 0.4,
                strokeWeight: 2,
                fillColor: "#0000FF",
                fillOpacity: 0.4
            });

            myCity.setMap(map);

            google.maps.event.addListener(map, 'click', function (event) {
                placeMarker(event.latLng);
            });
        }

        function placeMarker(location) {

            myCity.setCenter(location);
        }

        function decreaseRadius() {
            var radius = myCity.getRadius();
            radius -= 100;
            myCity.setRadius(radius);

            return false;
        }

        function increaseRadius() {
            var radius = myCity.getRadius();
            radius += 100;
            myCity.setRadius(radius);

            return false;
        }

        google.maps.event.addDomListener(window, 'load', initialize);
    </script>
<div runat="server" style="width: 600px;">
<div id="googleMap" style="width:500px;height:380px; float: left"></div>
<div style="float: right" class="btn-group">
    <asp:Button ID="increaseRadius" runat="server" Text="+" CssClass="btn btn-default" OnClientClick="return increaseRadius()" />
    <asp:Button ID="decreaseRadius" runat="server" Text="-" CssClass="btn btn-default" OnClientClick="return decreaseRadius()"/>
</div>
</div>