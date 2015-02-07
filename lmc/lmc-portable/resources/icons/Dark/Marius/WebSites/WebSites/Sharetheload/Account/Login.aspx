<%@ Page Title="Log in" Language="C#" MasterPageFile="~/Site.Master" AutoEventWireup="true" CodeFile="Login.aspx.cs" Inherits="Account_Login" Async="true" %>

<%@ Register Src="~/Account/OpenAuthProviders.ascx" TagPrefix="uc" TagName="OpenAuthProviders" %>

<asp:Content runat="server" ID="BodyContent" ContentPlaceHolderID="MainContent">
    <section id="loginForm" class="bg-light-gray">
        <div class="container">
            <h2><%: Title %></h2>
            <div class="row">
                <div class="form-horizontal">
                    <hr />
                    <asp:PlaceHolder runat="server" ID="ErrorMessage" Visible="false">
                        <p class="text-danger">
                            <asp:Literal runat="server" ID="FailureText" />
                        </p>
                    </asp:PlaceHolder>
                    <div class="form-group">
                        <asp:Label runat="server" AssociatedControlID="textBoxUserName" CssClass="col-md-2 control-label">Email address</asp:Label>
                        <div class="col-md-10">
                            <asp:TextBox runat="server" ID="textBoxUserName" CssClass="form-control" placeholder="Email address *"/>
                             <asp:CustomValidator id="CustomValidator2" runat="server"  CssClass="text-danger"
                                  OnServerValidate="UserValidate" 
                                  ControlToValidate="textBoxUserName" 
                                  ErrorMessage="The user doesn't exist">
                            </asp:CustomValidator>
                            <asp:RequiredFieldValidator runat="server" ControlToValidate="textBoxUserName"
                                CssClass="text-danger" ErrorMessage="The email address field is required." />
                        </div>
                    </div>
                    <div class="form-group">
                        <asp:Label runat="server" AssociatedControlID="textBoxPassword" CssClass="col-md-2 control-label">Password</asp:Label>
                        <div class="col-md-10">
                            <asp:TextBox runat="server" ID="textBoxPassword" TextMode="Password" CssClass="form-control" placeholder="Password *"/>
                            <asp:CustomValidator id="CustomValidator1" runat="server"  CssClass="text-danger"
                              OnServerValidate="PasswordValidate" 
                              ControlToValidate="textBoxPassword" 
                              ErrorMessage="The password is incorect">
                            </asp:CustomValidator>
                            <asp:RequiredFieldValidator ID="testing" runat="server" ControlToValidate="textBoxPassword" CssClass="text-danger" ErrorMessage="The password field is required." />
                        </div>
                    </div>
                    <div class="form-group">
                        <div class="col-md-offset-2 col-md-10">
                            <div class="checkbox">
                                <asp:CheckBox runat="server" ID="RememberMe" />
                                <asp:Label runat="server" AssociatedControlID="RememberMe">Remember me?</asp:Label>
                            </div>
                        </div>
                    </div>
                    <div class="form-group">
                        <div class="col-md-offset-2 col-md-10">
                            <asp:Button runat="server" OnClick="LogIn" Text="Log in" CssClass="btn btn-default" />
                        </div>
                    </div>
                </div>
                <p>
                    <asp:HyperLink runat="server" ID="RegisterHyperLink" ViewStateMode="Disabled">Register</asp:HyperLink>
                    if you don't have an account.
                </p>
            </div>
        </div>
    </section>

</asp:Content>

