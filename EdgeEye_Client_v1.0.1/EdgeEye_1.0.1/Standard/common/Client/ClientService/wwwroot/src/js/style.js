// left menu
(function($) {
  $.fn.jPushMenu = function(customOptions) {
    var o = $.extend({}, $.fn.jPushMenu.defaultOptions, customOptions);

    $('body').addClass(o.bodyClass);
    $(this).addClass('jPushMenuBtn');
    $(this).click(function() {
      var target         = '',
      push_direction     = '';

      if($(this).is('.'+o.showLeftClass)) {
        target         = '.cbp-spmenu-left';
        push_direction = 'toright';
      }

      $(this).toggleClass(o.activeClass);
      $(target).toggleClass(o.menuOpenClass);

      if($(this).is('.'+o.pushBodyClass)) {
        $('body').toggleClass( 'cbp-spmenu-push-'+push_direction );
      }

      // $('.jPushMenuBtn').not($(this)).toggleClass('disabled');
      // return false;
    });
    var jPushMenu = {
      close: function (o) {
        $('.jPushMenuBtn,body,.cbp-spmenu').removeClass('disabled active cbp-spmenu-open cbp-spmenu-push-toleft cbp-spmenu-push-toright');
      }
    }
    if(o.closeOnClickOutside) {
       $(document).click(function() { 
        jPushMenu.close();
       }); 

       $('.cbp-spmenu,.toggle-menu').click(function(e){ 
         e.stopPropagation(); 
       });
     }
  };
  $.fn.jPushMenu.defaultOptions = {
    bodyClass       : 'cbp-spmenu-push',
    activeClass     : 'menu-active',
    showLeftClass   : 'menu-left',
    menuOpenClass   : 'cbp-spmenu-open',
    pushBodyClass   : 'push-body',
    closeOnClickOutside: true
  };
})(jQuery);

/** toggle-menu **/
jQuery(document).ready(function($) {
  $('.toggle-menu').jPushMenu();
  $("#change_menu").click(function(){
    $('.menu_box navhide').slideToggle("slow");
  });
});

// left menu end 

// font max-width 
$(function(){
  var len = 26; // 超過26個字以"..."取代
  $(".fontMW").each(function(i){
      if($(this).text().length>len){
          $(this).attr("title",$(this).text());
          var text=$(this).text().substring(0,len-1)+"...";
          $(this).text(text);
      }
  });
  var len = 12; // 超過12個字以"..."取代
  $(".fontMW2").each(function(i){
      if($(this).text().length>len){
          $(this).attr("title",$(this).text());
          var text=$(this).text().substring(0,len-1)+"...";
          $(this).text(text);
      }
  });
});
// font max-width end