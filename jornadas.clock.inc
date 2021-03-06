<?php

function clock_bulk_import(){

    $default_start=strtotime('2/1/2015 00:00:00');
    $start=variable_get('clock_last_time_import',$default_start);


    for ($i=$start; $i < $start + (160*UN_DIA); $i=$i+UN_DIA) { 

      $operations[]=array('process_clock_data',array(1,$i));

    }
    variable_set('clock_last_time_import',$i);
    
 $batch = array(
    'operations' => $operations,
    'finished' => 'batch_clock_finished',
    
  );
    batch_set($batch);
        batch_process('jornadas-laborales');

}

function batch_clock_finished(){

}

function process_clock_data($jid,$start_time){
    $jornada=array();
    $personal=array();
    $data=get_fichajes_for_day($start_time);
    $last_time=$start_time;
    foreach ($data as  $values) {
        $values['real_time']=make_real_time($values);
        $values['jornada_time']=make_jornada_time($values);
        

        $values['day_stamp']=floor( $values['jornada_time'] /UN_DIA);

        $last_time=$values['real_time'];
        if(!isset($jornada[$values['day_stamp']])){
            if(!$jornada[$values['day_stamp']]=get_jid($values['real_time'])){
                $jornada[$values['day_stamp']]=jornada_create($values['real_time']);
                if($jornada[$values['day_stamp']]==False) continue;

            }   else {
             $jornada[$values['day_stamp']]=array_shift( $jornada[$values['day_stamp']]);
            }

        }
        if($values[2]=='E'){
            clock_new_recurrencia($values,$jornada[$values['day_stamp']]->jid);            
        } elseif($values[2]=='S'){
          clock_set_salida($values,$jornada[$values['day_stamp']]);
        }


    }
    return $last_time;
}

function clock_new_recurrencia($value,$jid,$start_confirmed=1){
        $personal=get_personal_by_legajo($value[3]);
        if(!$personal) return ;
        $recurrencia=new stdClass();
        $recurrencia->pid=$personal->pid;
        $recurrencia->jid=$jid;
        $recurrencia->start_time=$value['real_time'];
        $recurrencia->end_time=NULL;
        $recurrencia->work_type=($personal->work_type)?$personal->work_type:'nn';
        $recurrencia->start_confirmed=$start_confirmed;
        $recurrencia->present=1;
        $recurrencia->planta=0;
        
       try {
              drupal_write_record('personal_jornada',$recurrencia);
              
          } catch (Exception $e) {
             drupal_set_message( $e->getMessage());
          }

}
function clock_set_salida($value,$jornada){
        $personal=get_personal_by_legajo($value[3]);
        if(!$personal) return ;

        $recurrencia=get_last_recurrencias_by_jid($jornada->jid,$personal ->pid);
        if(isset($recurrencia[$personal ->pid])){
            db_update('personal_jornada')
                ->fields(array('end_time'=>$value['real_time'], 'end_confirmed'=>CLOCK_DATA_CONFIRMED))
                ->expression('time_total', ':real_time - start_time', array(':real_time' => $value['real_time']))
                ->condition('jpid',$recurrencia[$personal ->pid]->jpid)
                ->execute();
        } else {

          $recurrencia=new stdClass();
          $recurrencia->pid=$personal->pid;
          $recurrencia->jid=$jornada->jid;
          $recurrencia->start_time=$value['real_time'];
          $recurrencia->end_time=$value['real_time'];
          $recurrencia->work_type='xx';
          $recurrencia->start_confirmed=CLOCK_DATA_UNCONFIRMED;
          $recurrencia->end_confirmed=CLOCK_DATA_CONFIRMED;
          $recurrencia->present=1;
          $recurrencia->planta=0;

         try {
                drupal_write_record('personal_jornada',$recurrencia);
                
            } catch (Exception $e) {
               drupal_set_message( $e->getMessage());
            }

        }

}

function make_real_time($data){
    $time_str=substr($data[0], 5,11) .substr($data[1],16,9);
    return strtotime($time_str);
    $day=format_date(strtotime($data[0] . "-3") +10,'custom','m/d/Y ');
    $hour=format_date(strtotime($data[1]. "-3"),'custom','G:i:s');

    return strtotime($day . $hour); 

}


function make_jornada_time($data){
    $time_str=substr($data[0], 5,11) .'00:00';
    return strtotime($time_str);
    $day=format_date(strtotime($data[0] . "-3") +10,'custom','m/d/Y ');
    $hour='00:00';
    echo $day . $hour ."<br>";
    die();
    return strtotime($day . $hour); 

}

function get_fichajes_for_day($start_time){
    $day=date('m-d-y',$start_time);
    $hour=date('G:i:s',$start_time);
    $end_hour=date('G:i:s',($start_time + (15*UN_MINUTO)));
    return call_clock("fichaje/{$day}/{$hour}");


}


function call_clock($url,$query=array()){
  $data = drupal_http_build_query($query);
  $options = array(
                'method' => 'GET',
                'data' => $data,
                'timeout' => 15,
                'headers' => array('Content-Type' => 'application/json'),
        );
  $request = drupal_http_request(variable_get('clock_server','http://192.168.1.109:5000/'  ). $url  , $options);
  return json_decode($request->data);

}

function set_planta_now($jid){
  $return = '';
  $jornada=entity_load('data_jornadas',array($jid));

  $q=db_select('personal_jornada','p');
  $q->join('personal','pe','pe.pid=p.pid');
  $q->fields('p');
  $q->fields('pe',array('legajo','name'));
  $q->condition('planta',0);
  $q->condition('jid',$jid);
  $result=$q->execute();
  $i=0;
  while ($ocurencia = $result->fetchObject()) {
    if($ocurencia->start_time == NUll ){
      $ocurencia->start_time ="00 de dia";
    }
    if($ocurencia->end_time == NUll){
      $ocurencia->end_time ="24 de dia";

    } 
      continue;

    $time_sting=format_date($ocurencia->start_time,'custom','Y/m/d H:i:s');
    $endtime_string=format_date($ocurencia->end_time,'custom','Y/m/d H:i:s');
    $rq=db_select('v_fruitid','vf')->fields('vf',array('linea'))->condition('ingreso',$time_sting,'>=')->condition('ingreso',$endtime_string,'<=')
              ->condition('legajo',intval($ocurencia->legajo))-> groupBy('linea');

    
    $planta=$rq->execute()->fetchField();
    if ($planta) {
     db_update('personal_jornada')->fields(array('planta'=>$planta,'work_type'=>'Empacador')) ->condition('jpid',$ocurencia->jpid)->execute();
     $return .= "{$i}) {$ocurencia->legajo} {$ocurencia->name} : {$planta}<br/>";
    }
    //if($i>10) {break;}
    $i++;
  }
  return $return;
}

function set_planta($time){
 
  $time_string=date('Y-m-d G:i:s',$time);
  $endtime_string=date('Y-m-d G:i:s',$time + (UN_MINUTO*15));
  $last_prod=db_query("SELECT legajo, turno , MIN(UNIX_TIMESTAMP(ingreso)) AS pri, MAX(UNIX_TIMESTAMP(ingreso)) AS ult , ingreso
                        FROM v_fruitid
                        WHERE ingreso >= '".$time_sting ."' and  ingreso <= '".$endtime_string."'
                        GROUP BY legajo, turno ");
  $jornada=array_shift(get_jid($time,$delta=0));

  $jid=array_keys($jornada);
  $jid=$jid[0];
  $persona_activo=get_personal_activo_por_jornada($jid,'legajo');
  $personal=all_personal_by_legajo();

  while ($produccion = $last_prod->fetchObject) {
   
    if(!isset($persona_activo[$produccion->legajo])){
          $recurrencia=clock_empacador_new_recurrencia($personal[$produccion->legajo],$jid,$produccion->pri,0,CLOCK_DATA_UNCONFIRMED,$produccion->linea);
          $persona_activo[$produccion->legajo]=$recurrencia;
    }

    else 
      if (isset($persona_activo[$produccion->legajo]) and $persona_activo[$produccion->legajo]->planta==0){
        db_update('personal_jornada')->fields(array('planta'=>$produccion->linea)) ->condition('jpid',$persona_activo[$produccion->legajo]->jpid);
          continue;
    } 
    else 
      if (isset($persona_activo[$produccion->legajo]) and $persona_activo[$produccion->legajo]->planta!=$produccion->linea){

          $recurrencia=clock_empacador_new_recurrencia($personal[$produccion->legajo],$jid,$produccion->pri,0,CLOCK_DATA_UNCONFIRMED,$produccion->linea);
          $persona_activo[$produccion->legajo]=$recurrencia;
 
      }
    
  }


}

function clock_empacador_new_recurrencia($personal,$jid,$start_time,$end_time,$clock_state,$planta){

          $recurrencia=new stdClass();
          $recurrencia->pid=$personal->pid;
          $recurrencia->jid=$jid;
          $recurrencia->start_time=$start_time;
          $recurrencia->start_time=$end_time;
          $recurrencia->work_type='Empacador';
          $recurrencia->start_confirmed=$clock_state;
          $recurrencia->end_confirmed=$clock_state;
          $recurrencia->present=1;
          $recurrencia->planta=$panta;

          drupal_write_record('personal_jornada',$recurrencia);
          return $recurrencia;

}

function clock_import_personal(){
  $legajos=call_clock("legajos");
  foreach ($legajos as $legajo) {
      db_merge('personal')
      ->key(
        array(
          'legajo' => intval($legajo[0])
          )
        )
      ->fields(array(
            'legajo' => intval($legajo[0]),
            'name' => $legajo[1],
            'active' => 1,
            'change_date' => time(),
            'create_date' => time(),
            'create_uid' => 1
      ))
      ->execute();
  }

}

/*
         
         try {
                drupal_write_record('personal_jornada',$recurrencia);


*/