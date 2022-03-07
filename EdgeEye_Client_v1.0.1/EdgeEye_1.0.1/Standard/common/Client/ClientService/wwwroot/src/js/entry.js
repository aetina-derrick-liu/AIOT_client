import "bootstrap/dist/css/bootstrap.min.css";
import "bootstrap/dist/js/bootstrap.min.js";
import "font-awesome-webpack";
import "jvectormap";
import "jvectormap/jquery-jvectormap.css";
import "../css/layout/nav.css";
import "../css/components/font.css";
import "../css/components/buttons.css";
import "../css/components/cards.css";
import "../css/style.css";
import dt from'datatables.net';
import 'datatables.net-dt/css/jquery.dataTables.css';
import "jquery.cookie";

var Chart = require('chart.js');

import frame from "../frame.html";
import serviceUnavailable from "../ServiceUnavailable.html";
import logo_icap from "../images/logo-edgeeye.png";


$(document).ready(function(){
    document.getElementById("entry-main").innerHTML = frame;
    require("./api/pageswitcher");
});

$(document).on("service-unavailable", function(){
    document.getElementById("entry-main").innerHTML = serviceUnavailable;
    document.getElementById("logo_icap_nav").src = logo_icap;
    $.cookie("current_page", "serviceunavailable");
});