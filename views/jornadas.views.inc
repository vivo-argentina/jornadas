<?php
/**
* Implements hook_views_data_alter(&$data)().
*/
function jornadas_views_data_alter(&$data) {
    &$data['data_jornadas']['production_time']=array(
      'field' => array(
      'handler' => 'views_handler_production_time',
      'click sortable' => false,
       ),
    );

}

class views_handler_production_time extends views_handler_field {
  function render($values) {
  
   return 'hola';
  }
}