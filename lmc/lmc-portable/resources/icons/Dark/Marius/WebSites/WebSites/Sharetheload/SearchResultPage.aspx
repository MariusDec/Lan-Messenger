<%@ Page Language="C#" AutoEventWireup="true" MasterPageFile="~/MainPages/Main.master" CodeFile="SearchResultPage.aspx.cs" Inherits="SearchResultPage" %>
<%@ Register TagPrefix="uc" TagName="RequestControl" Src="~/Controls/RequestControl.ascx" %>
<%@ Reference Control="~/Controls/RequestControl.ascx" %>

<asp:Content ID="Content1" ContentPlaceHolderID="MainContent" runat="Server">

    <div class="container">
        <div class="row">
            <div class="panel panel-default" style="margin-top: 20px; width: 600px;">
                <div class="panel-heading">Results found: </div>
                <asp:Table ID="table" runat="server" class="table" />

            </div>
        </div>
    </div>

</asp:Content>