<%@ Page Title="" Language="C#" MasterPageFile="~/MainPages/Main.master" AutoEventWireup="true" CodeFile="FollowedRequests.aspx.cs" Inherits="Pages_FallowedRequests" %>
<%@ Register TagPrefix="uc" TagName="RequestControl" Src="~/Controls/RequestControl.ascx" %>
<%@ Reference Control="~/Controls/RequestControl.ascx" %>

<asp:Content ID="Content1" ContentPlaceHolderID="MainContent" Runat="Server">

<div class="container">
    <div class="row">
        <div class="panel panel-default" style="margin-top: 20px; width: 600px; ">
            <div class="panel-heading">Your request subscriptions</div>
            <asp:Table ID="table" runat="server" class="table"/>
            
        </div>
    </div>
</div>

</asp:Content>

