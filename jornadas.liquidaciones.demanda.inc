<?php

/*function lista_meses(){
    $table=array();

    $meses=db_query("SELECT DISTINCT MONTH(FROM_UNIXTIME(start_time)) AS mes ,YEAR(FROM_UNIXTIME(start_time)) AS anio FROM jornadas " );

    $i=0;
    foreach ($meses as $jornada) {
        $i++;
        $table['rows'][]=array($i,l($jornada ->mes . "/".$jornada ->anio , "jornadas/resumen/{$jornada ->mes}/{$jornada ->anio}"));  
    }
    if($_GET['csv']) theme('table_csv',$table);

    return theme('table',$table) . theme('link_csv');

}*/

function productividad_demanda_intervalo($form,&$form_state){
	
    $from['coment']=array('#markup'=>t('<h3>La productividad sera calculada dentro del intervalo.</h3>'));

    $from['start_time']=array('#type'=>'date', '#title'=>'Inicio');
    $from['end_time']=array('#type'=>'date', '#title'=>'Fin');

    $from['enviar']=array('#type'=>'submit', '#value'=>'Calcular');
    
    return $from;
}

function productividad_demanda_intervalo_validate($form,&$form_state){
	
  $form_state['values']['start_time']=mktime(0,0,0,$form_state['values']['start_time']['month'],$form_state['values']['start_time']['day'],$form_state['values']['start_time']['year']);
  $form_state['values']['end_time']=mktime(23,59,58,$form_state['values']['end_time']['month'],$form_state['values']['end_time']['day'],$form_state['values']['end_time']['year']);

	if($form_state['values']['start_time'] > $form_state['values']['end_time']){

		form_set_error("Periodo Incorrecto");
      drupal_set_message(t("<strong>Periodo Incorrecto</strong></br>La fecha final debe ser posterior a la inicial"), 'error');
	
	
	}

}

function productividad_demanda_intervalo_submit($form,&$form_state){
	
	unset($_GET['destination']);
	
	unset($form_state['rebuild']);  // if $form_state['rebuild'] == TRUE, the redirect will be ignored
	
	
	
   $form_state['redirect'] = array('jornadas/demanda/'.$form_state["values"]["start_time"].'/'.$form_state["values"]["end_time"]);//, array('fragment' => 'anchorid'));
   
    // array('fragment' => 'anchorid') is used because I needed redirection to an anchor in specific node	
	
}



function personal_demanda($inicio,$final){
	
	$t_inicio = date('d-m-Y',$inicio);
	$t_fin = date('d-m-Y',$final);
	
    $table=array();
    drupal_set_title("{$t_inicio} a {$t_fin}");



    $personal=db_query("SELECT p.name, ppj.pid, COUNT(j.jid) as  dias ,SUM(numeric_work) as jornadas,SUM(time_work) as tiempo , SUM(pjp.monto) as monto , ppj.monto * SUM(numeric_work) as sueldo , p.categoria, p.legajo ,SUM(time_extra) as time_extra
                        FROM planta_personal_jornada ppj
                        JOIN personal p ON p.pid=ppj.pid
                        JOIN jornadas j ON j.jid=ppj.jid
                        LEFT JOIN personal_jornada_productividad pjp ON (ppj.jid= pjp.jid AND ppj.pid=pjp.pid)
                        WHERE
                        j.start_time >= :inicio
                        AND
                        j.start_time < :fin
                        GROUP BY ppj.pid
                        " , array(':inicio'=>$inicio,':fin'=>$final));

    $i=0;
    foreach ($personal as $persona) {
        $i++;
        $h=intval($persona ->tiempo/3600);
        $m=intval(($persona ->tiempo/3600-$h)*60);
        
        $hx=intval($persona ->time_extra/3600);
        $mx=intval(($persona ->time_extra/3600-$hx)*60);
		  

        $table['rows'][]=array($i,"{$persona ->legajo} " ,l($persona ->name, "jornadas/demanda/{$inicio}/{$final}/{$persona ->pid}"), "{$persona ->categoria}",$persona ->dias,$persona ->jornadas,"$h horas $m minutos","$hx horas $mx minutos",number_format($persona ->monto,2,',','.'),number_format($persona ->sueldo,2,',','.'),
                    number_format($persona ->monto + $persona ->sueldo,2,',','.')
            );  
    }
    $table['header']=array('N','Legajo','Nombre','categoria','Dias','Jornadas','tiempo','extra','productividad','sueldo','total');
    if($_GET['csv']) theme('table_csv',$table);

    return theme('table',$table) .  theme('link_csv');

}
function personal_demanda_particular($inicio,$final,$pid){
	
	$t_inicio = date('d-m-Y',$inicio);
	$t_fin = date('d-m-Y',$final);

    $table=array();
    $personal=entity_load('data_personal',array($pid));

    drupal_set_title("{$personal[$pid]->name}, {$t_inicio} a {$t_fin}");
   $personal=db_query("SELECT ppj.jid, ppj.pid, j.start_time  ,numeric_work as jornadas,time_work as tiempo , ppj.work_type, pjp.monto as monto ,
                            ppj.monto * numeric_work as sueldo , p.categoria ,time_extra
                        FROM planta_personal_jornada ppj
                        JOIN personal p ON p.pid=ppj.pid
                        JOIN jornadas j ON j.jid=ppj.jid
                        LEFT JOIN personal_jornada_productividad pjp ON (ppj.jid= pjp.jid AND ppj.pid=pjp.pid)
                        WHERE 
                        j.start_time >= :inicio
                        AND
                        j.start_time < :fin
                        AND
                        ppj.pid = :pid
                        " , array(':inicio'=>$inicio,':fin'=>$final,':pid'=>$pid));


  $i=0;
  $productividad=0;
  $sueldo=0;
    foreach ($personal as $persona) {
        $i++;
        $h=intval($persona ->tiempo/3600);
        $m=intval(($persona ->tiempo/3600-$h)*60);

        $hx=intval($persona ->time_extra/3600);
        $mx=intval(($persona ->time_extra/3600-$hx)*60);

        $sueldo=$sueldo+$persona ->sueldo;
        $productividad=$productividad+$persona ->monto;
        $table['rows'][]=array($i,l(format_date($persona ->start_time,'custom','d/m/Y'),"jornadas/productividad/detalle_embalador/{$persona->jid}/{$persona->pid}"),$persona->work_type,$persona ->jornadas,"$h horas $m minutos","$hx horas $mx minutos",number_format($persona ->monto,2,',','.'),number_format($persona ->sueldo,2,',','.'),number_format($persona ->monto+ $persona ->sueldo,2,',','.'));  
    }
    $table['rows'][]=array('','','','','',number_format($productividad,2,',','.'),number_format($sueldo,2,',','.'),number_format($productividad+$sueldo,2,',','.'));  

    $table['header']=array('N','Dia','tarea','Jornadas','tiempo','extra','monto','sueldo','total');
    if($_GET['csv']) theme('table_csv',$table);

    return theme('table',$table) .  theme('link_csv');
    

}
