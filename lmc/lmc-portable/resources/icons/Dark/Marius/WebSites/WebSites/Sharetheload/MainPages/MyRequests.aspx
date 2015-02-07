<%@ Page Title="" Language="C#" MasterPageFile="~/MainPages/Main.master" AutoEventWireup="true" CodeFile="MyRequests.aspx.cs" Inherits="Pages_MyRequests" %>
<%@ Register TagPrefix="uc" TagName="RequestControl" Src="~/Controls/RequestControl.ascx" %>
<%@ Reference Control="~/Controls/RequestControl.ascx" %>

<asp:Content ID="Content1" ContentPlaceHolderID="MainContent" Runat="Server">

    <div class="container">
    <div class="row" >
        <%--<fieldset>
            <legend>Your requests!</legend>
            <asp:Table ID="table" runat="server" class="table"/>
            
        </fieldset>--%>
        <div class="panel panel-default" style="margin-top: 20px; width: 600px; ">
            <div class="panel-heading">Your requests!</div>
            <asp:Table ID="table" runat="server" class="table"/>
            
        </div>        
    </div>
</div>

</asp:Content>

