%check off
Program nameserverclient;

{ +----------------------------------------------------------------+
  | ** This is a VERY SIMPLE nameserver client. It allows lookup   |
  | ** of names and email addresses, no more. Uses TCPIP services. |
  | **                                                             |
  | ** Charley Kline, University of Illinois Computing Services.   |
  | ** 22 Oct 1987                                                 |
  +----------------------------------------------------------------+ }

%include CMALLCL

Label
    69;

Const
    nameserverhost = '80ae053a'x;  { garcon.cso.uiuc.edu }
    csnetmailboxprogramport = 105;
    toomany = 20;
    opentimeout = 10;  { seconds }
    LR_OK = 200;
    LR_AINFO = 503;
    LR_ABSENT = 508;

Var
    parameters: statusinfotype;
    note: notificationinfotype;
    i, tcprc: integer;
    l: string(133);
    them: internetaddresstype;
    therequest: string(132);
    errorp: Boolean;

Procedure asctoebc(var buf: char; buflen: integer); External;
Procedure ebctoasc(var buf: char; buflen: integer); External;


Function getline: integer;

    Var
        c: char;
        rc, i: integer;


    Function getnetchar(var c: char): integer;

        Static
            bufpos: integer;
            bufsiz: integer;
            buf: packed array[1..512] of char;

        Value
            bufpos := 1;
            bufsiz := 0;

        begin
            if bufpos > bufsiz then begin
                tcpwaitreceive(parameters.connection,
                   addr(buf[1]), 512, bufsiz);
                if bufsiz < 0 then begin
                    getnetchar := bufsiz;
                    bufpos := 1;
                    bufsiz := 0;
                    return
                    end;
                bufpos := 1
                end;
            c := buf[bufpos];
            bufpos := bufpos + 1;
            getnetchar := 0
            end;


    begin
        i := 1;
        l := '';
        while true do begin
            rc := getnetchar(c);
            if rc < 0 then begin
                getline := rc;
                return
                end;
            if (c = ASCIILF) | (i > 133) then begin
                asctoebc(l[1], i);
                getline := i;
                return
                end;
            l := l || str(c);
            i := i + 1
            end
        end;





Procedure submit;

    var
        buf: string(133);

    begin
        buf := therequest || '0A'xc;

        ebctoasc(buf[1], length(buf));

        tcpwaitsend(parameters.connection, addr(buf[1]),
                    length(buf), true, false, tcprc);
        if tcprc ^= ok then begin
            writeln('An error. TWSEND returns ', saycalre(tcprc));
            writeln('See a consultant with the above message.');
            goto 69
            end
        end;




Procedure getqueryresponse;
{
** Get the response from the nameserver. The first line says
** how many matched, subsequent lines are the returned responses
** themselves.
}
    var
        pos, theperson, currperson, code: integer;
        tok: alpha;

    begin
        currperson := 0;
        repeat
            pos := 1;
            if getline < 0 then begin
                writeln('Error reading from name server.');
                return
                end;
            token(pos, l, tok);
            if tok[1] = '#' then begin
                code := -100;
                continue
                end;
            if tok[1] = '-' then begin
                token(pos, l, tok);
                readstr(str(tok), code);
                code := -code;
                end
            else readstr(str(tok), code);
            if (code = -LR_OK) | (code = -LR_AINFO) | (code=-LR_ABSENT)
               then begin
                pos := pos + 1;       { Skip over colon }
                token(pos, l, tok);
                readstr(str(tok), theperson);
                if theperson <> currperson then begin
                    writeln('----------------------------------------')
                    currperson := theperson
                    end;
                writeln(delete(l, 1, pos+1));
                end
            else if code <> LR_OK then writeln(l)
            until code >= LR_OK;
        writeln('----------------------------------------')
        end;




Procedure getregularoldresponse;

    begin
        repeat
            if getline < 0 then begin
                writeln('Error reading from name server.');
                goto 69
                end
            else if (l[1] <> '#') then writeln(l)
            until l[1] in ['2'..'5']
    end;




begin  { Main program }

    termin(input);
    termout(output);


    {
    ** Establish connection with the TCPIP virtual machine.
    }
    initemulation(errorp);
    if errorp then begin
        writeln('ECMODE emulation failed - see a consultant.');
        goto 69
        end;
    begintcpip(tcprc);
    if tcprc <> ok then begin
        writeln('TCP/IP not there - notify operations.');
        writeln(saycalre(tcprc));
        halt
        end;

    handle(allnotes, tcprc);
    if tcprc <> ok then begin
        writeln('An error. HANOTES returns ', saycalre(tcprc));
        writeln('See a consultant with the above message.');
        goto 69
        end;


    {
    ** Try to connect to the nameserver.
    }
    with parameters do begin
        connection := unspecifiedconnection;
        openattempttimeout := opentimeout;
        security := defaultsecurity;
        compartment := defaultcompartment;
        precedence := defaultprecedence;
        connectionstate := tryingtoopen;
        localsocket.address := unspecifiedaddress;
        localsocket.port := unspecifiedport;
        foreignsocket.address := nameserverhost;
        foreignsocket.port := CsnetMailboxProgramPort;
        end;
    tcpwaitopen(parameters,tcprc);
    if tcprc ^= ok then begin
        writeln('Sorry, phone book not available - try later.');
        writeln(saycalre(tcprc));
        goto 69
        end;


    {
    ** Build the NS query message and ship it off to the
    ** nameserver.
    }
    repeat
        read(therequest);
        submit;
        if (length(therequest)>4) & (substr(therequest,1,5) = 'query')
            then getqueryresponse
        else getregularoldresponse
        until therequest = 'quit';
    tcpclose(parameters.connection, tcprc);
69:
    endtcpip;
    dropemulation
    end.




