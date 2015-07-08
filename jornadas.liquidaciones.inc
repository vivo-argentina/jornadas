<?php

function lista_meses(){
    $table=array();

    $meses=db_query("SELECT DISTINCT MONTH(FROM_UNIXTIME(start_time)) AS mes ,YEAR(FROM_UNIXTIME(start_time)) AS anio FROM jornadas " );

    $i=0;
    foreach ($meses as $jornada) {
        $i++;
        $table['rows'][]=array($i,l($jornada ->mes . "/".$jornada ->anio , "jornadas/resumen/{$jornada ->mes}/{$jornada ->anio}"));  
    }
    return theme('table',$table);

}


function personal_mensual($month,$year=2015){
    $table=array();
    drupal_set_title("{$month}/{$year}");



    $personal=db_query("SELECT p.name, ppj.pid, COUNT(j.jid) as  dias ,SUM(numeric_work) as jornadas,SUM(time_work) as tiempo , SUM(pjp.monto) as monto , ppj.monto * SUM(numeric_work) as sueldo , p.categoria 
                        FROM planta_personal_jornada ppj
                        JOIN personal p ON p.pid=ppj.pid
                        JOIN jornadas j ON j.jid=ppj.jid
                        JOIN personal_jornada_productividad pjp ON (ppj.jid= pjp.jid AND ppj.pid=pjp.pid)
                        WHERE 
                        MONTH(FROM_UNIXTIME(j.start_time))=:month
                        AND YEAR(FROM_UNIXTIME(j.start_time))=:year
                        GROUP BY ppj.pid
                        " , array(':month'=>$month,':year'=>$year));

    $i=0;
    foreach ($personal as $persona) {
        $i++;
        $h=intval($persona ->tiempo/3600);
        $m=intval(($persona ->tiempo/3600-$h)*60);
        $table['rows'][]=array($i,l($persona ->name, "jornadas/resumen/{$month}/{$year}/{$persona ->pid}"). " ({$persona ->categoria})",$persona ->dias,$persona ->jornadas,"$h horas $m minutos",number_format($persona ->monto,2,',','.'),number_format($persona ->sueldo,2,',','.'),
                    number_format($persona ->monto + $persona ->sueldo,2,',','.')
            );  
    }
    $table['header']=array('N','Nombre','Dias','Jornadas','tiempo','productividad','sueldo','total');
    return theme('table',$table);

}
function personal_resumen_mensual($month,$year,$pid){

    $table=array();
    $personal=entity_load('data_personal',array($pid));

    drupal_set_title("{$personal[$pid]->name}, {$month}/{$year}");
   $personal=db_query("SELECT ppj.jid, ppj.pid, j.start_time  ,numeric_work as jornadas,time_work as tiempo , ppj.work_type, pjp.monto as monto ,
                            ppj.monto * numeric_work as sueldo , p.categoria 
                        FROM planta_personal_jornada ppj
                        JOIN personal p ON p.pid=ppj.pid
                        JOIN jornadas j ON j.jid=ppj.jid
                        JOIN personal_jornada_productividad pjp ON (ppj.jid= pjp.jid AND ppj.pid=pjp.pid)
                        WHERE 
                        MONTH(FROM_UNIXTIME(j.start_time))=:month
                        AND YEAR(FROM_UNIXTIME(j.start_time))=:year
                        and ppj.pid = :pid
                        " , array(':month'=>$month,':year'=>$year,':pid'=>$pid));


  $i=0;
  $productividad=0;
  $sueldo=0;
    foreach ($personal as $persona) {
        $i++;
        $h=intval($persona ->tiempo/3600);
        $m=intval(($persona ->tiempo/3600-$h)*60);
        $sueldo=$sueldo+$persona ->sueldo;
        $productividad=$productividad+$persona ->monto;
        $table['rows'][]=array($i,l(format_date($persona ->start_time,'custom','d/m/Y'),"jornadas/productividad/detalle_embalador/{$persona->jid}/{$persona->pid}"),$persona->work_type,$persona ->jornadas,"$h horas $m minutos",number_format($persona ->monto,2,',','.'),number_format($persona ->sueldo,2,',','.'),number_format($persona ->monto+ $persona ->sueldo,2,',','.'));  
    }
    $table['rows'][]=array('','','','','',number_format($productividad,2,',','.'),number_format($sueldo,2,',','.'),number_format($productividad+$sueldo,2,',','.'));  

    $table['header']=array('N','Dia','tarea','Jornadas','tiempo','monto','sueldo','total');
    return theme('table',$table);
    

}