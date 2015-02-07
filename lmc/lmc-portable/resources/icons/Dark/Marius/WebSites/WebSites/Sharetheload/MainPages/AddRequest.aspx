<%@ Page Title="Add Request" Language="C#" MasterPageFile="~/MainPages/Main.master" AutoEventWireup="true" CodeFile="AddRequest.aspx.cs" Inherits="AddRequest" %>

<asp:Content ID="Content1" ContentPlaceHolderID="MainContent" runat="Server">

    <script src="http://maps.googleapis.com/maps/api/js"></script>
	<script type="text/javascript">
		function saveDate() {
			$("#<%= hiddenFieldDate.ClientId %>").val($("#datePickerDeadline").val());
		}
	</script>
    
    <script>
        var map
        var myCity

        function initialize() {
            var radiusChanged = '<%=Session["radiusChanged"]%>';

            if (radiusChanged == 'False') {
                var latitude = '<%=Session["latitude"]%>';
                var longitude = '<%=Session["longitude"]%>';
                var reqCenter = new google.maps.LatLng(latitude, longitude);

                var mapProp = {
                    center: reqCenter,
                    zoom: 10,
                    mapTypeId: google.maps.MapTypeId.ROADMAP
                };

                map = new google.maps.Map(document.getElementById("googleMap"), mapProp);

                myCity = new google.maps.Circle({
                    center: reqCenter,
                    radius: 5000,
                    strokeColor: "#0000FF",
                    strokeOpacity: 0.4,
                    strokeWeight: 2,
                    fillColor: "#0000FF",
                    fillOpacity: 0.4
                });

                myCity.setMap(map);
            }
        }

        function decreaseRadius() {
            var radius = myCity.getRadius();
            radius -= 100;
            myCity.setRadius(radius);
            document.cookie = "Radius=" + radius;
            return false;
        }

        function increaseRadius() {
            var radius = myCity.getRadius();
            radius += 100;
            myCity.setRadius(radius);
            document.cookie = "Radius=" + radius;
            return false;
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
                <hr />
                    <div class="col-md-6">
                <asp:ValidationSummary runat="server" CssClass="text-danger" />
                <div class="form-group">
                    <asp:Label runat="server" AssociatedControlID="RequestTitle" CssClass="col-md-2 control-label">Title: </asp:Label>
                    <div class="col-md-10">
                        <asp:TextBox runat="server" ID="RequestTitle" CssClass="form-control" placeholder="Title *" />
                        <asp:RequiredFieldValidator runat="server" ControlToValidate="RequestTitle"
                            CssClass="text-danger" ErrorMessage="The request title is required." />
                    </div>
                </div>
                <div class="form-group">
                    <asp:Label runat="server" AssociatedControlID="ProductName" CssClass="col-md-2 control-label">Product name: </asp:Label>
                    <div class="col-md-10">
                        <asp:TextBox runat="server" ID="ProductName" CssClass="form-control" placeholder="Product name *" />
                        <asp:RequiredFieldValidator runat="server" ControlToValidate="ProductName"
                            CssClass="text-danger" ErrorMessage="The product name is required." />
                    </div>
                </div>
                <div class="form-group">
                    <asp:Label runat="server" AssociatedControlID="ProductDesc" CssClass="col-md-2 control-label">Product description: </asp:Label>
                    <div class="col-md-10">
                        <asp:TextBox runat="server" ID="ProductDesc" CssClass="form-control" placeholder="Product description *" />
                        <asp:RequiredFieldValidator runat="server" ControlToValidate="ProductDesc"
                            CssClass="text-danger" ErrorMessage="The product description is required." />
                    </div>
                </div>
                <div class="form-group">
                    <asp:Label runat="server" CssClass="col-md-2 control-label">Category: </asp:Label>
                    <div class="col-md-10">
                        <select class="form-control" runat="server" id="CategoryList" name="Category">
                            <option value="0">All</option>
                            <option value="1">Games</option>
                            <option value="2">IT</option>
                            <option value="3">Misc</option>
                            <option value="4">Tools</option>
                        </select>
                    </div>
                </div>
                <div class="form-group">
                    <asp:Label runat="server" AssociatedControlID="Price" CssClass="col-md-2 control-label">Estimated price: </asp:Label>
                    <div class="col-md-10">
                        <asp:TextBox runat="server" ID="Price" CssClass="form-control" placeholder="Estimated price *" />
                        <asp:RequiredFieldValidator runat="server" ControlToValidate="Price"
                            CssClass="text-danger" ErrorMessage="The estimated price is required." />
                    </div>
                </div>
                <div class="form-group">
                    <asp:Label runat="server" AssociatedControlID="MinFollow" CssClass="col-md-2 control-label">Minimum followers: </asp:Label>
                    <div class="col-md-10">
                        <asp:TextBox runat="server" ID="MinFollow" CssClass="form-control" placeholder="Minimum followers *" />
                        <asp:RequiredFieldValidator runat="server" ControlToValidate="MinFollow"
                            CssClass="text-danger" ErrorMessage="The minimum followers number is required." />
                    </div>
                </div>
                <div class="form-group">
                    <asp:Label runat="server" AssociatedControlID="MaxFollow" CssClass="col-md-2 control-label">Maximum followers: </asp:Label>
                    <div class="col-md-10">
                        <asp:TextBox runat="server" ID="MaxFollow" CssClass="form-control" placeholder="Maximum followers *" />
                        <asp:RequiredFieldValidator runat="server" ControlToValidate="MaxFollow"
                            CssClass="text-danger" ErrorMessage="The maximum followers number is required." />
                    </div>
                </div>
                <div class="form-group">
                    <asp:Label runat="server" CssClass="col-md-2 control-label">Deadline: </asp:Label>
                    <div class="col-md-10">
                        <input id="datePickerDeadline" data-provide="datepicker-inline" class="datepicker" onblur="saveDate()">
						<asp:HiddenField ID="hiddenFieldDate" runat=server" />
                    </div>
                </div>

                <div class="form-group">
                    <div class="col-md-offset-2 col-md-10">
                        <asp:Button runat="server" Text="Add request" CssClass="btn btn-default" OnClick="AddRequest_Click" />
                    </div>
                </div>
                        </div>

                <div class="col-md-6" style="margin-top: 50px;">
                    <asp:UpdatePanel ID="UpdatePanel1" runat="server">
                        <ContentTemplate>
                            <div runat="server" style="width: 600px;">
                                <div id="googleMap" style="width: 500px; height: 380px; float: left"></div>
                                <div style="float: right" class="btn-group">
                                    <asp:Button ID="increaseRadius" runat="server" Text="+" CssClass="btn btn-default" OnClientClick="return increaseRadius()" />
                                    <asp:Button ID="decreaseRadius" runat="server" Text="-" CssClass="btn btn-default" OnClientClick="return decreaseRadius()" />
                                </div>
                            </div>
                            <asp:Label runat="server" id="radiusLabel" Text=""></asp:Label>
                        </ContentTemplate>
                    </asp:UpdatePanel>
                </div>

            </div>
            </div>
        </div>
    </section>
</asp:Content>

