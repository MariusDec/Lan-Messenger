<%@ Control Language="C#" AutoEventWireup="true" CodeFile="RequestControl.ascx.cs" Inherits="Controls_RequestControl" %>

<div style="height: 200px; width: 606px">
    <div style="padding-top: 20px; padding-left:20px;">

        <div class="row"><asp:Label ID="titleLabel" runat="server" Font-Bold="True" Font-Size="Larger" Text=""></asp:Label></div>
        <div class="row"></div>
        <div><asp:Label ID="descriptionLabel" runat="server" Text=""></asp:Label></div>
        <div style="width: 200px;" class="row">
            <asp:Label ID="startLabel" runat="server" Text="Started: " style="float: left"></asp:Label>
            <asp:Label ID="startDate" runat="server" Text="test" style="float: right"></asp:Label>
        </div>
        <div style="width: 200px;" class="row">
            <asp:Label ID="endLabel" runat="server" Text="Deadline: " style="float: left"></asp:Label>
            <asp:Label ID="endDate" runat="server" Text="test" style="float: right"></asp:Label>
        </div>
        <div class="row">
            <asp:Label ID="currentlySubscribed" runat="server" Text="Subscribed: "></asp:Label>
            <asp:Label ID="subscribers" runat="server" Text=""></asp:Label>
        </div>

        <div class="row"></div>
        <div class="row" style="height: 28px">
            <asp:Button ID="subscribeButton" runat="server" CssClass="btn btn-default" Text="Subscribe" CausesValidation="false" style="width:77px;float:center;padding-left: 0px;padding-right: 0px;" OnClick="subscribeButton_Click" Width="77px"/>
        </div>
    </div>
</div>
