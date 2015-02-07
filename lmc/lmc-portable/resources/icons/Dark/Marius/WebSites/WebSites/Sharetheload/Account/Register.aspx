<%@ Page Title="Register" Language="C#" MasterPageFile="~/Site.Master" AutoEventWireup="true" CodeFile="Register.aspx.cs" Inherits="Account_Register" %>

<asp:Content runat="server" ID="BodyContent" ContentPlaceHolderID="MainContent">

   <script src="http://ajax.googleapis.com/ajax/libs/jquery/1.11.1/jquery.min.js" type="text/javascript"></script>
    <script src="http://maps.googleapis.com/maps/api/js"></script>
    <script type="text/javascript" >
        var map
        var myLocation
        var infowindow
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
            document.cookie = "Latitude=" + location.lat();
            document.cookie = "Longitude=" + location.lng();
            document.getElementById('<%=latitudeLabel.ClientID %>').innerHTML = '' + location.lat();
            document.getElementById('<%=longitudeLabel.ClientID %>').innerHTML = '' + location.lng();
           
        }

        google.maps.event.addDomListener(window, 'load', initialize);
    </script>

    <section id="registerForm" class="bg-light-gray">
        <div class="container">
            <h2><%: Title %></h2>
            <p class="text-danger">
                <asp:Literal runat="server" ID="ErrorMessage" />
            </p>

            <div class="form-horizontal">
                <div class="="row">
                    <div class="col-md-6">
                <hr />
                <asp:ValidationSummary runat="server" CssClass="text-danger" />
             <div class="form-group">
                    <asp:Label runat="server" AssociatedControlID="textBoxFirstName" CssClass="col-md-2 control-label">First Name:</asp:Label>
                    <div class="col-md-10">
                        <asp:TextBox runat="server" ID="textBoxFirstName" CssClass="form-control" placeholder="First Name *" />
                        <asp:RequiredFieldValidator runat="server" ControlToValidate="textBoxFirstName"
                            CssClass="text-danger" ErrorMessage="The first name is required." />
                    </div>
                </div>
                <div class="form-group">
                    <asp:Label runat="server" AssociatedControlID="textBoxLastName" CssClass="col-md-2 control-label">Last Name :</asp:Label>
                    <div class="col-md-10">
                        <asp:TextBox runat="server" ID="textBoxLastName" CssClass="form-control" placeholder="Last Name *" />
                        <asp:RequiredFieldValidator runat="server" ControlToValidate="textBoxLastName"
                            CssClass="text-danger" ErrorMessage="The last name is required." />
                    </div>
                </div>
                <div class="form-group">
                    <asp:Label runat="server" AssociatedControlID="textBoxEmail" CssClass="col-md-2 control-label">Email:</asp:Label>
                    <div class="col-md-10">
                        <asp:TextBox runat="server" ID="textBoxEmail" CssClass="form-control" placeholder="Email *" />
                        <asp:RequiredFieldValidator runat="server" ControlToValidate="textBoxEmail"
                            CssClass="text-danger" ErrorMessage="The email field is required." />
                        <asp:CustomValidator id="CustomValidator1" runat="server"  CssClass="text-danger"
                              OnServerValidate="EmailValidate" 
                              ControlToValidate="textBoxEmail" 
                              ErrorMessage="The email is already used.">
                            </asp:CustomValidator>
                    </div>
                </div>
                <div class="form-group">
                    <asp:Label runat="server" AssociatedControlID="textBoxPassword" CssClass="col-md-2 control-label">Password</asp:Label>
                    <div class="col-md-10">
                        <asp:TextBox runat="server" ID="textBoxPassword" TextMode="Password" CssClass="form-control" placeholder="Password *" />
                        <asp:RequiredFieldValidator runat="server" ControlToValidate="textBoxPassword"
                            CssClass="text-danger" ErrorMessage="The password field is required." />
                    </div>
                </div>
                <div class="form-group">
                    <asp:Label runat="server" AssociatedControlID="ConfirmPassword" CssClass="col-md-2 control-label">Confirm Password</asp:Label>
                    <div class="col-md-10">
                        <asp:TextBox runat="server" ID="ConfirmPassword" TextMode="Password" CssClass="form-control" placeholder="Confirm Password *" />
                        <asp:RequiredFieldValidator runat="server" ControlToValidate="ConfirmPassword"
                            CssClass="text-danger" Display="Dynamic" ErrorMessage="The confirm password field is required." />
                        <asp:CompareValidator runat="server" ControlToCompare="textBoxPassword" ControlToValidate="ConfirmPassword"
                            CssClass="text-danger" Display="Dynamic" ErrorMessage="The password and confirmation password do not match." />
                    </div>
                </div>
                <div class="form-group">
                    <asp:Label runat="server" AssociatedControlID="textBoxPhone" CssClass="col-md-2 control-label">Phone:</asp:Label>
                    <div class="col-md-10">
                        <asp:TextBox runat="server" ID="textBoxPhone" CssClass="form-control" placeholder="Phone *" />
                        <asp:RequiredFieldValidator runat="server" ControlToValidate="textBoxPhone"
                            CssClass="text-danger" ErrorMessage="Phone number is required." />
                    </div>
                </div>

                <div class="form-group">
                    <asp:Label runat="server" AssociatedControlID="textBoxCountry" CssClass="col-md-2 control-label">Country:</asp:Label>
                    <div class="col-md-10">
                        <asp:TextBox runat="server" ID="textBoxCountry" CssClass="form-control" placeholder="Country *" />
                        <asp:RequiredFieldValidator runat="server" ControlToValidate="textBoxCountry"
                            CssClass="text-danger" ErrorMessage="Country is required." />
                    </div>
                </div>

                <div class="form-group">
                    <asp:Label runat="server" AssociatedControlID="textBoxState" CssClass="col-md-2 control-label">State:</asp:Label>
                    <div class="col-md-10">
                        <asp:TextBox runat="server" ID="textBoxState" CssClass="form-control" placeholder="State *" />
                        <asp:RequiredFieldValidator runat="server" ControlToValidate="textBoxState"
                            CssClass="text-danger" ErrorMessage="State is required." />
                    </div>
                </div>

                <div class="form-group">
                    <asp:Label runat="server" AssociatedControlID="textBoxCity" CssClass="col-md-2 control-label">City:</asp:Label>
                    <div class="col-md-10">
                        <asp:TextBox runat="server" ID="textBoxCity" CssClass="form-control" placeholder="City *" />
                        <asp:RequiredFieldValidator runat="server" ControlToValidate="textBoxCity"
                            CssClass="text-danger" ErrorMessage="Country is required." />
                    </div>
                </div>

                <div></div>
                <div class="form-group">
                    <div class="col-md-offset-2 col-md-10">
                        <asp:Button runat="server" OnClick="CreateUser_Click" Text="Register" CssClass="btn btn-default" />
                    </div>
                </div>
                 </div>
                    <div class="col-md-6" style="margin-top: 50px;">
                        <asp:UpdatePanel ID="UpdatePanel1" runat="server">
                            <ContentTemplate>
                                <div id="googleMap" style="width:500px;height:380px; float: left"></div>
                                 
                                <asp:Label runat="server" ID="latitudeLabel" Text=""></asp:Label>
                                <asp:Label runat="server" ID="longitudeLabel" Text=""></asp:Label>
                            </ContentTemplate>
                        </asp:UpdatePanel>
                    </div>
               </div>
            </div>
    </section>
        </div>
       
</asp:Content>

