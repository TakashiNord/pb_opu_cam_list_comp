//////////////////////////////////////////////////////////////////////////////
//
//  opu_cam10_list_comp.cpp
//
//  Description:
//      Contains Unigraphics entry points for the application.
//
//////////////////////////////////////////////////////////////////////////////

#define _CRT_SECURE_NO_DEPRECATE 1

//  Include files
#include <uf.h>
#include <uf_exit.h>
#include <uf_ui.h>
/*
#if ! defined ( __hp9000s800 ) && ! defined ( __sgi ) && ! defined ( __sun )
# include <strstream>
  using std::ostrstream;
  using std::endl;
  using std::ends;
#else
# include <strstream.h>
#endif
#include <iostream.h>
*/
#include "opu_cam10_list_comp.h"


#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include <uf.h>
#include <uf_ui.h>
#include <uf_modl.h>
#include <uf_part.h>
#include <uf_attr.h>
#include <uf_cfi.h>
#include <uf_assem.h>
#include <uf_disp.h>
#include <uf_obj.h>
#include <uf_object_types.h>
#include <uf_ugmgr.h>

//****************************************************************************

#include <uf_defs.h>
#include <uf_exit.h>
#include <uf_styler.h>
#include <uf_mb.h>
#include "opu_cam_set_part_name.h"

/* The following definition defines the number of callback entries */
/* in the callback structure:                                      */
/* UF_STYLER_callback_info_t CAM_cbs */
#define CAM_CB_COUNT ( 6 + 1 ) /* Add 1 for the terminator */

/*--------------------------------------------------------------------------
The following structure defines the callback entries used by the
styler file.  This structure MUST be passed into the user function,
UF_STYLER_create_dialog along with CAM_CB_COUNT.
--------------------------------------------------------------------------*/
static UF_STYLER_callback_info_t CAM_cbs[CAM_CB_COUNT] =
{
 {UF_STYLER_DIALOG_INDEX, UF_STYLER_CONSTRUCTOR_CB  , 0, CAM_construct_cb},
 {UF_STYLER_DIALOG_INDEX, UF_STYLER_APPLY_CB        , 0, CAM_apply_cb},
 {UF_STYLER_DIALOG_INDEX, UF_STYLER_BACK_CB         , 0, CAM_back_cb},
 {CAM_ACTION_SPECIFY    , UF_STYLER_ACTIVATE_CB     , 1, CAM_action_specify_cb},
 {CAM_STR_PART_NAME     , UF_STYLER_ACTIVATE_CB     , 0, CAM_part_name_cb},
 {CAM_BLO_ACTION        , UF_STYLER_ACTIVATE_CB     , 0, CAM_blo_act_cb},
 {UF_STYLER_NULL_OBJECT, UF_STYLER_NO_CB, 0, 0 }
};

/*--------------------------------------------------------------------------
UF_MB_styler_actions_t contains 4 fields.  These are defined as follows:
 
Field 1 : the name of your dialog that you wish to display.
Field 2 : any client data you wish to pass to your callbacks.
Field 3 : your callback structure.
Field 4 : flag to inform menubar of your dialog location.  This flag MUST  
          match the resource set in your dialog!  Do NOT ASSUME that changing 
          this field will update the location of your dialog.  Please use the 
          UIStyler to indicate the position of your dialog.
--------------------------------------------------------------------------*/
static UF_MB_styler_actions_t actions[] = {
    { "opu_cam_set_part_name.dlg",  NULL,   CAM_cbs,  UF_MB_STYLER_IS_NOT_TOP },
    { NULL,  NULL,  NULL,  0 } /* This is a NULL terminated list */
};


//****************************************************************************

char *nameAttr[]={"OPU_PART_NAME","OPU_PART_NAME_TIME",NULL} ;

#define UF_CALL(X) (report( __FILE__, __LINE__, #X, (X)))

static int report( char *file, int line, char *call, int irc)
{
  if (irc)
  {
     char    messg[133];
     printf("%s, line %d:  %s\n", file, line, call);
     (UF_get_fail_message(irc, messg)) ?
       printf("    returned a %d\n", irc) :
       printf("    returned error %d:  %s\n", irc, messg);
  }
  return(irc);
}

/**
-------------------------------------
**/
static int mask_for_components(UF_UI_selection_p_t select, void *type)
{
    UF_UI_mask_t  mask = { UF_component_type, 0, 0 };

    if (!UF_CALL(UF_UI_set_sel_mask(select, UF_UI_SEL_MASK_CLEAR_AND_ENABLE_SPECIFIC, 1, &mask)))
        return (UF_UI_SEL_SUCCESS);
    else return (UF_UI_SEL_FAILURE);
}

/**
-------------------------------------
**/
static tag_t select_a_component(char *prompt)
{
    int     resp;
    double  cp[3];
    tag_t   object, view;

    UF_CALL(UF_UI_select_with_single_dialog(prompt, "",
        UF_UI_SEL_SCOPE_ANY_IN_ASSEMBLY, mask_for_components, NULL, &resp,
        &object, cp, &view));

    if (resp == UF_UI_OBJECT_SELECTED || resp == UF_UI_OBJECT_SELECTED_BY_NAME)
    {
        UF_CALL(UF_DISP_set_highlight(object, 0));
        return object;
    }
    else return NULL_TAG;

}

/**
-------------------------------------
**/
static void set_attributes(tag_t object)
{
    char  part_name[132+1];
    char  part_number[UF_UGMGR_PARTNO_SIZE+1];
    char  refset_name[30+1];
    char  instance_name[30+1];
    double  origin[3];
    double  csys_matrix[9];
    double  transform[4][4];
    char  part_revision[UF_UGMGR_PARTREV_SIZE+1];
    char  part_file_type[UF_UGMGR_FTYPE_SIZE+1];
    char  part_file_name[UF_UGMGR_FNAME_SIZE+1];

    char  part_name_set[UF_ATTR_MAX_STRING_LEN+1];
    char  *ppart_name_set;
    int   ir3 ;
    int   resp ;
    tag_t prt ;
    UF_ATTR_value_t value;


    UF_CALL( UF_ASSEM_ask_component_data(object,part_name,refset_name,instance_name,origin,csys_matrix,transform) );

    printf("part_name=%s\n\t refset_name=%s\n\t instance_name=%s\n",part_name,refset_name,instance_name);

    UF_CALL( UF_UGMGR_decode_part_filename(part_name,part_number,part_revision, part_file_type,part_file_name) );

    printf("part_number=%s\n",part_number);
    printf("part_revision=%s\n",part_revision);
    printf("part_file_type=%s\n",part_file_type);
    printf("part_file_name=%s\n",part_file_name);

    part_name_set[0]='\0';
    sprintf(part_name_set,"%s_cam_%s",part_number,part_revision);
    printf("part_name_set=%s\n",part_name_set);

    ppart_name_set=part_name_set;
    resp = uc1600 ("введите имя части=", ppart_name_set, &ir3 );
    printf("ppart_name_set=%s\n",ppart_name_set);
    if (resp!=3 || resp!=5) return ;

    prt = UF_PART_ask_display_part() ; //UF_ASSEM_ask_work_part()

    /*  Assign a string attribute to the object */
    value.type = UF_ATTR_string;
    value.value.string = ppart_name_set;
    UF_CALL(UF_ATTR_assign(prt, nameAttr[0] , value));
    /*  Assign a date and time attribute to the  */
    value.type = UF_ATTR_time;
    UF_CALL(uc4583("", "", value.value.time));    /* Init to current date and time */
    UF_CALL(UF_ATTR_assign(prt, nameAttr[1], value));

    part_name_set[0]='\0';
    sprintf(part_name_set,"%s=%s",nameAttr[0],ppart_name_set);
    UF_UI_set_status( part_name_set );


}


/*****************************************************************************/
int _main_loadDll( void )
{
    int  response   = 0;
    int errorCode;
    char *dialog_name=actions->styler_file;
    char env_names[][25]={
    "UGII_USER_DIR" ,
    "UGII_SITE_DIR" ,
    "UGII_VENDOR_DIR" ,
    "USER_UFUN" ,
    "UGII_INITIAL_UFUN_DIR" ,
    "UGII_TMP_DIR" ,
    "HOME" ,
    "TMP" } ;
 int i ;
 char *path , envpath[133] , dlgpath[255] , info[133];
 int status ;

 path = (char *) malloc(133+10);

 errorCode=-1;
  for (i=0;i<7;i++) {

    envpath[0]='\0';
    path=envpath;
    UF_translate_variable(env_names[i], &path);
    if (path!=NULL) {

       /*1*/
       dlgpath[0]='\0';
       strcpy(dlgpath,path); strcat(dlgpath,"\\application\\"); strcat(dlgpath,dialog_name);
       UF_print_syslog(dlgpath,FALSE);

       // работа с файлом
       UF_CFI_ask_file_exist (dlgpath, &status );
       if (!status) { errorCode=0; break ; }

       /*2*/
       dlgpath[0]='\0';
       strcpy(dlgpath,path); strcat(dlgpath,"\\"); strcat(dlgpath,dialog_name);
       UF_print_syslog(dlgpath,FALSE);

       // работа с файлом
       UF_CFI_ask_file_exist (dlgpath, &status );
       if (!status) { errorCode=0; break ; }

     } else { //if (envpath!=NULL)
      info[0]='\0'; sprintf (info,"Переменная %s - не установлена \n ",env_names[i]);
      UF_print_syslog(info,FALSE);
     }
  } // for

 if (errorCode!=0) {
    info[0]='\0'; sprintf (info,"Don't load %s  \n ",dialog_name);
    uc1601 (info, TRUE );
  } else {
       if ( ( errorCode = UF_STYLER_create_dialog ( dlgpath,
           CAM_cbs,      /* Callbacks from dialog */
           CAM_CB_COUNT, /* number of callbacks*/
           NULL,        /* This is your client data */
           &response ) ) != 0 )
        {
              /* Get the user function fail message based on the fail code.*/
              PrintErrorMessage( errorCode );
         }
  }

 return(errorCode);
}



static void cam10(void)
{

    int module_id;
    UF_ask_application_module(&module_id);
    if (UF_APP_CAM!=module_id) {
       uc1601("Запуск DLL - производится из модуля обработки\n - 2005г.",1);
       return ;
    }

    if (NULL_TAG==UF_PART_ask_display_part()) {
      uc1601("Cam-часть не активна.....\n программа прервана.",1);
      return ;
    }

    if (0!=_main_loadDll()) {

       tag_t  comp ;

       while ((comp = select_a_component("Select a component")) != NULL_TAG)
       {
           set_attributes(comp);
       }

    }

}


int _construct_cb ( int dialog_id )
{
  UF_STYLER_item_value_type_t data  ;
  int irc ;
  char  part_name_set[UF_ATTR_MAX_STRING_LEN+1];
  tag_t prt ;
  UF_ATTR_value_t value;
  int l , indx = 0;
  char  title[UF_ATTR_MAX_TITLE_LEN+1];

  part_name_set[0]='\0';

  prt = UF_PART_ask_display_part() ; //UF_ASSEM_ask_work_part()

  // UF_ATTR_string
  while (!UF_CALL(UF_ATTR_cycle(prt, &indx, UF_ATTR_any, title, &value))
        && indx)
  {
    	if (0==strcmp(title,nameAttr[0])) {
       strcpy(part_name_set, value.value.string);
       //sprintf (part_name_set,"%s\0",value.value.string );
       printf("Consruct:%s=%s\n",nameAttr[0], part_name_set);
       l=(int)strlen(part_name_set);
       if (l<=1) {
      	 UF_CALL(UF_ATTR_delete(prt,UF_ATTR_string,nameAttr[0]));
         UF_CALL(UF_ATTR_delete(prt,UF_ATTR_time  ,nameAttr[1]));
         UF_UI_set_status( "" );
       }
       UF_free(value.value.string);
       break ;
    	}
    	if (0==strcmp(title,nameAttr[1])) { ;	}
   }

  data.item_attr=UF_STYLER_VALUE;
  data.item_id=CAM_STR_PART_NAME ;
  data.value.string=part_name_set;
  irc=UF_STYLER_set_value(dialog_id,&data);

  UF_STYLER_free_value (&data) ;

  return(0);
}



int _action_specify_cb ( int dialog_id )
{
  UF_STYLER_item_value_type_t data  ;
  int irc ;

    tag_t comp ;
    char  part_name[132+1];
    char  part_number[UF_UGMGR_PARTNO_SIZE+1];
    char  refset_name[30+1];
    char  instance_name[30+1];
    double  origin[3];
    double  csys_matrix[9];
    double  transform[4][4];
    char  part_revision[UF_UGMGR_PARTREV_SIZE+1];
    char  part_file_type[UF_UGMGR_FTYPE_SIZE+1];
    char  part_file_name[UF_UGMGR_FNAME_SIZE+1];

    char  part_name_set[UF_ATTR_MAX_STRING_LEN+1];

/********************************/

  while ((comp = select_a_component("Select a component")) != NULL_TAG)
  {

    UF_CALL( UF_ASSEM_ask_component_data(comp,part_name,refset_name,instance_name,origin,csys_matrix,transform) );

    printf("part_name=%s\n\t refset_name=%s\n\t instance_name=%s\n",part_name,refset_name,instance_name);

    UF_CALL( UF_UGMGR_decode_part_filename(part_name,part_number,part_revision, part_file_type,part_file_name) );

    printf("part_number=%s\n",part_number);
    printf("part_revision=%s\n",part_revision);
    printf("part_file_type=%s\n",part_file_type);
    printf("part_file_name=%s\n",part_file_name);

    part_name_set[0]='\0';
    sprintf(part_name_set,"%s_cam_%s",part_number,part_revision);
    printf("part_name_set=%s\n",part_name_set);

    data.item_attr=UF_STYLER_VALUE;
    data.item_id=CAM_STR_PART_NAME ;
    data.value.string = part_name_set;
    irc=UF_STYLER_set_value(dialog_id,&data);

    break ; // прерываем выполнение

  }

  UF_STYLER_free_value (&data) ;

  return (0);
}

int _action_set_cb ( int dialog_id )
{
  UF_STYLER_item_value_type_t data  ;
  int irc ;
  char  part_name[132];
  char  part_name_set[UF_ATTR_MAX_STRING_LEN+1];
  tag_t prt ;
  int   but , indx = 0;
//  char  title[UF_ATTR_MAX_TITLE_LEN+1];
  UF_ATTR_value_t value;

  data.item_attr=UF_STYLER_VALUE;
  data.item_id=CAM_STR_PART_NAME ;
  irc=UF_STYLER_ask_value(dialog_id,&data);
  part_name_set[0]='\0';
  sprintf (part_name_set,"%s",data.value.string );

  data.reason=UF_STYLER_ACTIVATE_REASON;
  data.item_id=CAM_BLO_ACTION;
  data.indicator=UF_STYLER_INTEGER_VALUE;
  irc=UF_STYLER_ask_value(dialog_id,&data);
  but=data.value.integer;

  UF_STYLER_free_value (&data) ;

  prt = UF_PART_ask_display_part() ; //UF_ASSEM_ask_work_part()

  if (but==1) {
    /*  Assign a string attribute to the object */
    value.type = UF_ATTR_string;
    value.value.string = part_name_set;
    UF_CALL(UF_ATTR_assign(prt, nameAttr[0] , value));
    /*  Assign a date and time attribute to the  */
    value.type = UF_ATTR_time;
    UF_CALL(uc4583("", "", value.value.time));    /* Init to current date and time */
    UF_CALL(UF_ATTR_assign(prt, nameAttr[1], value));

    part_name[0]='\0';
    sprintf(part_name,"%s=%s",nameAttr[0],part_name_set);
    UF_UI_set_status( part_name );
  }

  if (but!=1) {

    /*
    while (!UF_CALL(UF_ATTR_cycle(prt, &indx, UF_ATTR_any, title, &value))
        && indx)
    {
    	if (0==strcmp(title,nameAttr[0])) { ;	}
    	if (0==strcmp(title,nameAttr[1])) { ;	}
    }
    */
    UF_CALL(UF_ATTR_delete(prt,UF_ATTR_string,nameAttr[0]));
    UF_CALL(UF_ATTR_delete(prt,UF_ATTR_time  ,nameAttr[1]));
    part_name[0]='\0';
    sprintf(part_name,"%s - delete",nameAttr[0]);
    UF_UI_set_status( part_name );
  }

  return (0);
}


//----------------------------------------------------------------------------
//  Activation Methods
//----------------------------------------------------------------------------

//  Explicit Activation
//      This entry point is used to activate the application explicitly, as in
//      "File->Execute UG/Open->User Function..."
extern "C" DllExport void ufusr( char *parm, int *returnCode, int rlen )
{
    /* Initialize the API environment */
    int errorCode = UF_initialize();

    if ( 0 == errorCode )
    {
        /* TODO: Add your application code here */
        cam10();

        /* Terminate the API environment */
        errorCode = UF_terminate();
    }

    /* Print out any error messages */
    PrintErrorMessage( errorCode );
    *returnCode=0;
}

//----------------------------------------------------------------------------
//  Utilities
//----------------------------------------------------------------------------

// Unload Handler
//     This function specifies when to unload your application from Unigraphics.
//     If your application registers a callback (from a MenuScript item or a
//     User Defined Object for example), this function MUST return
//     "UF_UNLOAD_UG_TERMINATE".
extern "C" int ufusr_ask_unload( void )
{
     /* unload immediately after application exits*/
     return ( UF_UNLOAD_IMMEDIATELY );

     /*via the unload selection dialog... */
     /*return ( UF_UNLOAD_SEL_DIALOG );   */
     /*when UG terminates...              */
     /*return ( UF_UNLOAD_UG_TERMINATE ); */
}

/*--------------------------------------------------------------------------
You have the option of coding the cleanup routine to perform any housekeeping
chores that may need to be performed.  If you code the cleanup routine, it is
automatically called by Unigraphics.
--------------------------------------------------------------------------*/
extern void ufusr_cleanup (void)
{
    return;
}

/* PrintErrorMessage
**
**     Prints error messages to standard error and the Unigraphics status
**     line. */
static void PrintErrorMessage( int errorCode )
{
    if ( 0 != errorCode )
    {
        /* Retrieve the associated error message */
        char message[133];
        UF_get_fail_message( errorCode, message );

        /* Print out the message */
        UF_UI_set_status( message );

        fprintf( stderr, "%s\n", message );
    }
}



/*-------------------------------------------------------------------------*/
/*---------------------- UIStyler Callback Functions ----------------------*/
/*-------------------------------------------------------------------------*/

/* -------------------------------------------------------------------------
 * Callback Name: CAM_construct_cb
 * This is a callback function associated with an action taken from a
 * UIStyler object.
 *
 * Input: dialog_id   -   The dialog id indicate which dialog this callback
 *                        is associated with.  The dialog id is a dynamic,
 *                        unique id and should not be stored.  It is
 *                        strictly for the use in the NX Open API:
 *                               UF_STYLER_ask_value(s)
 *                               UF_STYLER_set_value
 *        client_data -   Client data is user defined data associated
 *                        with your dialog.  Client data may be bound
 *                        to your dialog with UF_MB_add_styler_actions
 *                        or UF_STYLER_create_dialog.
 *        callback_data - This structure pointer contains information
 *                        specific to the UIStyler Object type that
 *                        invoked this callback and the callback type.
 * -----------------------------------------------------------------------*/
int CAM_construct_cb ( int dialog_id,
             void * client_data,
             UF_STYLER_item_value_type_p_t callback_data)
{
     /* Make sure User Function is available. */
     if ( UF_initialize() != 0)
          return ( UF_UI_CB_CONTINUE_DIALOG );

     /* ---- Enter your callback code here ----- */
     _construct_cb ( dialog_id );

     UF_terminate ();

    /* Callback acknowledged, do not terminate dialog */
    return (UF_UI_CB_CONTINUE_DIALOG);
    /* A return value of UF_UI_CB_EXIT_DIALOG will not be accepted    */
    /* for this callback type.  You must continue dialog construction.*/

}


/* -------------------------------------------------------------------------
 * Callback Name: CAM_apply_cb
 * This is a callback function associated with an action taken from a
 * UIStyler object.
 *
 * Input: dialog_id   -   The dialog id indicate which dialog this callback
 *                        is associated with.  The dialog id is a dynamic,
 *                        unique id and should not be stored.  It is
 *                        strictly for the use in the NX Open API:
 *                               UF_STYLER_ask_value(s)
 *                               UF_STYLER_set_value
 *        client_data -   Client data is user defined data associated
 *                        with your dialog.  Client data may be bound
 *                        to your dialog with UF_MB_add_styler_actions
 *                        or UF_STYLER_create_dialog.
 *        callback_data - This structure pointer contains information
 *                        specific to the UIStyler Object type that
 *                        invoked this callback and the callback type.
 * -----------------------------------------------------------------------*/
int CAM_apply_cb ( int dialog_id,
             void * client_data,
             UF_STYLER_item_value_type_p_t callback_data)
{
     /* Make sure User Function is available. */
     if ( UF_initialize() != 0)
          return ( UF_UI_CB_CONTINUE_DIALOG );

     /* ---- Enter your callback code here ----- */

     UF_terminate ();

    /* Callback acknowledged, do not terminate dialog                 */
    /* A return value of UF_UI_CB_EXIT_DIALOG will not be accepted    */
    /* for this callback type.  You must respond to your apply button.*/
    return (UF_UI_CB_CONTINUE_DIALOG);

}


/* -------------------------------------------------------------------------
 * Callback Name: CAM_back_cb
 * This is a callback function associated with an action taken from a
 * UIStyler object.
 *
 * Input: dialog_id   -   The dialog id indicate which dialog this callback
 *                        is associated with.  The dialog id is a dynamic,
 *                        unique id and should not be stored.  It is
 *                        strictly for the use in the NX Open API:
 *                               UF_STYLER_ask_value(s)
 *                               UF_STYLER_set_value
 *        client_data -   Client data is user defined data associated
 *                        with your dialog.  Client data may be bound
 *                        to your dialog with UF_MB_add_styler_actions
 *                        or UF_STYLER_create_dialog.
 *        callback_data - This structure pointer contains information
 *                        specific to the UIStyler Object type that
 *                        invoked this callback and the callback type.
 * -----------------------------------------------------------------------*/
int CAM_back_cb ( int dialog_id,
             void * client_data,
             UF_STYLER_item_value_type_p_t callback_data)
{
     /* Make sure User Function is available. */
     if ( UF_initialize() != 0)
          return ( UF_UI_CB_CONTINUE_DIALOG );

     /* ---- Enter your callback code here ----- */

     UF_terminate ();

    /* Callback acknowledged, terminate dialog.            */
    /* It is STRONGLY recommended that you exit your       */
    /* callback with UF_UI_CB_EXIT_DIALOG in a back call   */
    /* back rather than UF_UI_CB_CONTINUE_DIALOG.          */
    return (UF_UI_CB_EXIT_DIALOG);


}


/* -------------------------------------------------------------------------
 * Callback Name: CAM_action_specify_cb
 * This is a callback function associated with an action taken from a
 * UIStyler object.
 *
 * Input: dialog_id   -   The dialog id indicate which dialog this callback
 *                        is associated with.  The dialog id is a dynamic,
 *                        unique id and should not be stored.  It is
 *                        strictly for the use in the NX Open API:
 *                               UF_STYLER_ask_value(s)
 *                               UF_STYLER_set_value
 *        client_data -   Client data is user defined data associated
 *                        with your dialog.  Client data may be bound
 *                        to your dialog with UF_MB_add_styler_actions
 *                        or UF_STYLER_create_dialog.
 *        callback_data - This structure pointer contains information
 *                        specific to the UIStyler Object type that
 *                        invoked this callback and the callback type.
 * -----------------------------------------------------------------------*/
int CAM_action_specify_cb ( int dialog_id,
             void * client_data,
             UF_STYLER_item_value_type_p_t callback_data)
{
     /* Make sure User Function is available. */
     if ( UF_initialize() != 0)
          return ( UF_UI_CB_CONTINUE_DIALOG );

     /* ---- Enter your callback code here ----- */
     _action_specify_cb ( dialog_id );

     UF_terminate ();

    /* Callback acknowledged, do not terminate dialog */
    return (UF_UI_CB_CONTINUE_DIALOG);

    /* or Callback acknowledged, terminate dialog.    */
    /* return ( UF_UI_CB_EXIT_DIALOG );               */

}

/* -------------------------------------------------------------------------
 * Callback Name: CAM_part_name_cb
 * This is a callback function associated with an action taken from a
 * UIStyler object.
 *
 * Input: dialog_id   -   The dialog id indicate which dialog this callback
 *                        is associated with.  The dialog id is a dynamic,
 *                        unique id and should not be stored.  It is
 *                        strictly for the use in the NX Open API:
 *                               UF_STYLER_ask_value(s)
 *                               UF_STYLER_set_value
 *        client_data -   Client data is user defined data associated
 *                        with your dialog.  Client data may be bound
 *                        to your dialog with UF_MB_add_styler_actions
 *                        or UF_STYLER_create_dialog.
 *        callback_data - This structure pointer contains information
 *                        specific to the UIStyler Object type that
 *                        invoked this callback and the callback type.
 * -----------------------------------------------------------------------*/
int CAM_part_name_cb ( int dialog_id,
             void * client_data,
             UF_STYLER_item_value_type_p_t callback_data)
{
     /* Make sure User Function is available. */
     if ( UF_initialize() != 0)
          return ( UF_UI_CB_CONTINUE_DIALOG );

     /* ---- Enter your callback code here ----- */

     UF_terminate ();

    /* Callback acknowledged, do not terminate dialog */
    return (UF_UI_CB_CONTINUE_DIALOG);

    /* or Callback acknowledged, terminate dialog.    */
    /* return ( UF_UI_CB_EXIT_DIALOG );               */

}


/* -------------------------------------------------------------------------
 * Callback Name: CAM_blo_act_cb
 * This is a callback function associated with an action taken from a
 * UIStyler object.
 *
 * Input: dialog_id   -   The dialog id indicate which dialog this callback
 *                        is associated with.  The dialog id is a dynamic,
 *                        unique id and should not be stored.  It is
 *                        strictly for the use in the NX Open API:
 *                               UF_STYLER_ask_value(s)
 *                               UF_STYLER_set_value
 *        client_data -   Client data is user defined data associated
 *                        with your dialog.  Client data may be bound
 *                        to your dialog with UF_MB_add_styler_actions
 *                        or UF_STYLER_create_dialog.
 *        callback_data - This structure pointer contains information
 *                        specific to the UIStyler Object type that
 *                        invoked this callback and the callback type.
 * -----------------------------------------------------------------------*/
int CAM_blo_act_cb ( int dialog_id,
             void * client_data,
             UF_STYLER_item_value_type_p_t callback_data)
{
     /* Make sure User Function is available. */
     if ( UF_initialize() != 0)
          return ( UF_UI_CB_CONTINUE_DIALOG );

     /* ---- Enter your callback code here ----- */
     _action_set_cb ( dialog_id );

     UF_terminate ();

    /* Callback acknowledged, do not terminate dialog */
    return (UF_UI_CB_CONTINUE_DIALOG);

    /* or Callback acknowledged, terminate dialog.    */
    /* return ( UF_UI_CB_EXIT_DIALOG );               */

}


