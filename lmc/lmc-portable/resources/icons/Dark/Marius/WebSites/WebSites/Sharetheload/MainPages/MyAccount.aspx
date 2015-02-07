<%@ Page Title="" Language="C#" MasterPageFile="~/MainPages/Main.master" AutoEventWireup="true" CodeFile="MyAccount.aspx.cs" Inherits="Pages_MyAccount" %>

<asp:Content ID="Content1" ContentPlaceHolderID="MainContent" Runat="Server">

        <section id="myAccountSection" class="bg-light-gray">

        <div class="container">
            <p><b>Account information:</b></p>
            <div class="row">
                <div class="form-horizontal">

                    <div class="form-group">
                        <asp:Label runat="server" AssociatedControlID="UserFirstName" CssClass="col-md-2 control-label">User firstname:</asp:Label>
                        <div class="col-md-10">
                            <asp:TextBox runat="server" ID="UserFirstName" CssClass="form-control" placeholder="Firstname *"/>
                            <asp:RequiredFieldValidator runat="server" ControlToValidate="UserFirstName"
                                CssClass="text-danger" ErrorMessage="The user firstname field is required." />
                        </div>
                    </div>

                    <div class="form-group">
                        <asp:Label runat="server" AssociatedControlID="UserLastName" CssClass="col-md-2 control-label">User lastname:</asp:Label>
                        <div class="col-md-10">
                            <asp:TextBox runat="server" ID="UserLastName" CssClass="form-control" placeholder="Lastname *"/>
                            <asp:RequiredFieldValidator runat="server" ControlToValidate="UserLastName"
                                CssClass="text-danger" ErrorMessage="The user lastname field is required." />
                        </div>
                    </div>

                    <div class="form-group">
                        <asp:Label runat="server" AssociatedControlID="UserPassword" CssClass="col-md-2 control-label">Password</asp:Label>
                        <div class="col-md-10">
                            <asp:TextBox runat="server" ID="UserPassword" CssClass="form-control" placeholder="Password *"/>
                            <asp:RequiredFieldValidator runat="server" ControlToValidate="UserPassword" 
                                CssClass="text-danger" ErrorMessage="The password field is required." />
                            <p>Password can be changed by editing it text.</p>
                        </div>
                    </div>

                    <div class="form-group">
                        <asp:Label runat="server" AssociatedControlID="UserEmail" CssClass="col-md-2 control-label">Email:</asp:Label>
                        <div class="col-md-10">
                            <asp:TextBox runat="server" ID="UserEmail" CssClass="form-control" placeholder="Email *"/>
                            <asp:RequiredFieldValidator runat="server" ControlToValidate="UserEmail"
                                CssClass="text-danger" ErrorMessage="The email field is required." />
                        </div>
                    </div>

                    <div class="form-group">
                        <asp:Label runat="server" AssociatedControlID="UserPhone" CssClass="col-md-2 control-label">Phone:</asp:Label>
                        <div class="col-md-10">
                            <asp:TextBox runat="server" ID="UserPhone" CssClass="form-control" placeholder="Phone *"/>
                            <asp:RequiredFieldValidator runat="server" ControlToValidate="UserPhone"
                                CssClass="text-danger" ErrorMessage="The phone field is required." />
                        </div>
                    </div>
                </div>
            </div>

            <br><br>

            <p><b>Address information:</b></p>
            <div class="row">
                <div class="form-horizontal">

                    <div class="form-group">
                        <asp:Label runat="server" AssociatedControlID="AddressCountry" CssClass="col-md-2 control-label">Country:</asp:Label>
                        <div class="col-md-10">
                            <asp:TextBox runat="server" ID="AddressCountry" CssClass="form-control" placeholder="Country *"/>
                            <asp:RequiredFieldValidator runat="server" ControlToValidate="AddressCountry"
                                CssClass="text-danger" ErrorMessage="The country field is required." />
                        </div>
                    </div>

                    <div class="form-group">
                        <asp:Label runat="server" AssociatedControlID="AddressState" CssClass="col-md-2 control-label">State:</asp:Label>
                        <div class="col-md-10">
                            <asp:TextBox runat="server" ID="AddressState" CssClass="form-control" placeholder="State *"/>
                            <asp:RequiredFieldValidator runat="server" ControlToValidate="AddressState"
                                CssClass="text-danger" ErrorMessage="The state field is required." />
                        </div>
                    </div>

                    <div class="form-group">
                        <asp:Label runat="server" AssociatedControlID="AddressCity" CssClass="col-md-2 control-label">City:</asp:Label>
                        <div class="col-md-10">
                            <asp:TextBox runat="server" ID="AddressCity" CssClass="form-control" placeholder="City *"/>
                            <asp:RequiredFieldValidator runat="server" ControlToValidate="AddressCity" 
                                CssClass="text-danger" ErrorMessage="The city field is required." />
                        </div>
                    </div>                    

                    <div class="form-group">
                        <div class="col-md-offset-2 col-md-10">
                            <asp:Button runat="server" OnClick="Modify_Data" Text="Modify" CssClass="btn btn-default" />
                        </div>
                    </div>
                </div>
            </div>
        </div>
    </section>

</asp:Content>

