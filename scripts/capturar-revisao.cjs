// Compara o código anterior com a revisão atual, mantendo estado e escala da captura.
const fs=require('node:fs'),cp=require('node:child_process'),path=require('node:path');
const {chromium}=require(process.env.PLAYWRIGHT_MODULE||'playwright');
const raiz=path.resolve(__dirname,'..'),saida=path.join(raiz,'Review');
(async()=>{fs.mkdirSync(saida,{recursive:true});const navegador=await chromium.launch({executablePath:process.env.CHROMIUM||undefined,args:['--no-sandbox']});try{
 for(const versao of ['antes','depois']){
  let html;if(versao==='antes'){try{html=cp.execFileSync('git',['show','e56e628:voxbox.html'],{cwd:raiz,encoding:'utf8',stdio:['ignore','pipe','ignore']})}catch{console.log('Sem histórico do commit base; capturas anteriores já fornecidas foram mantidas.');continue}}else html=fs.readFileSync(path.join(raiz,'voxbox.html'),'utf8');
  const pagina=await navegador.newPage({viewport:{width:1300,height:850},deviceScaleFactor:2});pagina.on('pageerror',erro=>{throw erro});await pagina.setContent(html);await pagina.evaluate(()=>document.fonts.ready);await pagina.evaluate(()=>carregarPreset(library.find(p=>p.id==='default')));
  await pagina.addStyleTag({content:'#needle{transform:rotate(12deg)!important}'});await pagina.waitForTimeout(350);
  await pagina.locator('#face').screenshot({path:path.join(saida,'painel-'+versao+'.png')});await pagina.locator('.meter').screenshot({path:path.join(saida,'vu-'+versao+'.png')});await pagina.locator('#preset').click();await pagina.locator('#presetBrowser').screenshot({path:path.join(saida,'presets-'+versao+'.png')});await pagina.close();
 }
 }finally{await navegador.close()}})().catch(erro=>{console.error(erro);process.exitCode=1});
