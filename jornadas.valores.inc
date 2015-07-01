<?php

function valores_table(){
    $q=db_query('SELECT DISTINCT start_time,end_time FROM valores_perido');
    $table=array();
    foreach ($q as $value) {
      $table['rows'][]=array(l(format_date($value->start_time) . " " . format_date($value->end_time), "jornadas/valores/perido/{$value->start_time}/{$value->end_time}"));
    }
    return theme('table',$table);
}
function valores_jornada($form,&$form_state,$start_time,$end_time){
    if(!isset($form_state['start_time'])){
        $form_state['start_time']=$start_time;
        $form_state['end_time']=$end_time;

    }

    $form['start_time'] = array(
      '#type' => 'date', 
      '#title' =>'inicio', 
      '#default_value' => array('day'=>date('j',$start_time),'month'=> date('n',$start_time),'year'=> date('Y',$start_time)), 
      '#required' => true,
    );
    $form['end_time'] = array(
      '#type' => 'date', 
      '#title' =>'Fin', 
      '#default_value' => array('day'=>date('j',$end_time),'month'=> date('n',$end_time),'year'=> date('Y',$end_time)), 
      '#required' => true,
    );


    $valores=get_valores_by_time($start_time,$end_time);
    $form['horasdia'] = array(
      '#type' => 'fieldset', 
      '#title' => t('Horas por dia'), 
      '#tree' => true, 
    );
    $horas=array('lunes'=>1,'martes'=>2,'miercoles'=>3,'jueves'=>4,'viernes'=>5,'sabado'=>6,'domingo'=>7,);

    foreach ($horas as $key => $value) {
        $form['horasdia'][$key] = array(
          '#type' => 'textfield', 
          '#title' => $key, 
          '#default_value' =>($valores['horasdia'][$value])?$valores['horasdia'][$value]:0, 
          '#size' => 5, 
          '#maxlength' => 5, 
          '#required' => TRUE,

    );


    }
    $form['montos'] = array(
      '#type' => 'fieldset', 
      '#title' => t('MOntos productividad'), 
      '#tree' => true, 
    );


    $form['montos']['porcaja'] = array(
      '#type' => 'textfield', 
      '#title' => 'monto productivdad por caja embalador', 
      '#default_value' =>($valores['montos']['porcaja'])?$valores['montos']['porcaja']:2.79, 
      '#size' => 5, 
      '#maxlength' => 5, 
      '#required' => TRUE,

    );

    $form['montos']['porcajaresto'] = array(
      '#type' => 'textfield', 
      '#title' => 'monto productivdad por caja resto de planta', 
      '#default_value' =>($valores['montos']['porcaja'])?$valores['montos']['porcajaresto']:2.79, 
      '#size' => 5, 
      '#maxlength' => 5, 
      '#required' => TRUE,

    );

    $form['volumenes'] = array(
      '#type' => 'fieldset', 
      '#title' => t('Volumenes de produccion'), 
      '#tree' => true, 
    );


    $form['volumenes']['mnz'] = array(
      '#type' => 'textfield', 
      '#title' => 'minimo de cajas manzana', 
      '#default_value' =>($valores['volumenes']['mnz'])?$valores['volumenes']['mnz']:114, 
      '#size' => 5, 
      '#maxlength' => 5, 
      '#required' => TRUE,

    );

    $form['volumenes']['pera'] = array(
      '#type' => 'textfield', 
      '#title' => 'minimo de cajas pera', 
      '#default_value' =>($valores['volumenes']['pera'])?$valores['volumenes']['pera']:96, 
      '#size' => 5, 
      '#maxlength' => 5, 
      '#required' => TRUE,

    );

    $form['submit'] = array('#type' => 'submit', '#value' => t('ok'));

    
    return $form;
}
function valores_jornada_validate($form,&$form_state){
//array('year' => 2007, 'month' => 2, 'day'=> 15)

    $form_state['values']['start_time']=mktime(0,0,0,$form_state['values']['start_time']['month'],$form_state['values']['start_time']['day'],$form_state['values']['start_time']['year']);
    $form_state['values']['end_time']=mktime(0,0,0,$form_state['values']['end_time']['month'],$form_state['values']['end_time']['day'],$form_state['values']['end_time']['year']);

}
function valores_jornada_submit($form,&$form_state){
    if($form_state['start_time']){
        db_delete('valores_perido')->condition('start_time',$form_state['start_time'])->condition('end_time',$form_state['end_time'])->execute();
    }
    $horas=array('lunes'=>1,'martes'=>2,'miercoles'=>3,'jueves'=>4,'viernes'=>5,'sabado'=>6,'domingo'=>7,);

    foreach ($horas as $key => $value) {
        db_insert('valores_perido')->fields(array(
            'grupo'=>'horasdia',
            'codigo' => $value,
            'valor' => $form_state['values']['horasdia'][$key],
            'start_time' => $form_state['values']['start_time'],
            'end_time' => $form_state['values']['end_time']

        )) ->execute();
    }    

        db_insert('valores_perido')->fields(array(
            'grupo'=>'montos',
            'codigo' => 'porcaja',
            'valor' => $form_state['values']['montos']['porcaja'],
            'start_time' => $form_state['values']['start_time'],
            'end_time' => $form_state['values']['end_time']

        )) ->execute();

        db_insert('valores_perido')->fields(array(
            'grupo'=>'montos',
            'codigo' => 'porcajaresto',
            'valor' => $form_state['values']['montos']['porcajaresto'],
            'start_time' => $form_state['values']['start_time'],
            'end_time' => $form_state['values']['end_time']

        )) ->execute();

        db_insert('valores_perido')->fields(array(
            'grupo'=>'volumenes',
            'codigo' => 'mnz',
            'valor' => $form_state['values']['volumenes']['mnz'],
            'start_time' => $form_state['values']['start_time'],
            'end_time' => $form_state['values']['end_time']

        )) ->execute();
        db_insert('valores_perido')->fields(array(
            'grupo'=>'volumenes',
            'codigo' => 'pera',
            'valor' => $form_state['values']['volumenes']['pera'],
            'start_time' => $form_state['values']['start_time'],
            'end_time' => $form_state['values']['end_time']

        )) ->execute();
}