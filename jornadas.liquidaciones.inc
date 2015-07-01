<?php
function personal_mensual($month,$year=2015){
    $table=array();
    drupal_set_title("{$month}/{$year}");
    $personal=db_select('personal_jornada','pj');
    $personal->join('personal','p','p.pid=pj.pid');
    $personal->fields('p',array('name','pid'))
    ->groupBy('p.pid');
    $personal->where ('MONTH(FROM_UNIXTIME(start_time)) = :m',array(':m' =>$month));
    $personal->where ('YEAR(FROM_UNIXTIME(start_time)) = :y',array(':y' =>$year));

    $result = $personal->execute();
    $i=0;
    while ($persona = $result->fetchObject()) {
        $i++;
        $table['rows'][]=array($i,l($persona ->name, "jornada/resumen/{$month}/{$year}/{$persona ->pid}"));  
    }

    return theme('table',$table);

}
function personal_resumen_mensual($month,$year,$pid){
//    $personal=db_select('personal_jornada','pj')->join('personal','p','p.pid=pj.pid')->fields('p',array('name','pid'))->condition('MONTH(FROM_UNIXTIME(start_time))',$month)->condition('MONTH(FROM_UNIXTIME(start_time))',$year)->condition('pid'=>$pid)->execute();
    

}